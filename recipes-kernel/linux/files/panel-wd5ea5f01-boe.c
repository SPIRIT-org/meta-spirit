// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct wd5ea5f01 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;

	struct gpio_desc *reset_gpio;
	struct gpio_desc *te_gpio;
	struct gpio_desc *panel_enable_gpio;

	struct regulator *vddio;
	struct regulator *vci;
	struct regulator *avdd;
	struct regulator *elvdd;

	bool prepared;
	bool enabled;
};

static inline struct wd5ea5f01 *to_wd5ea5f01(struct drm_panel *panel)
{
	return container_of(panel, struct wd5ea5f01, panel);
}

/* Active-low reset: LOW = asserted */
static void wd5ea5f01_reset(struct wd5ea5f01 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(200);

	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(200);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(200);
}

static int wd5ea5f01_dsi_on(struct wd5ea5f01 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;

	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xf1, 0x5a, 0x5a);

	mipi_dsi_dcs_exit_sleep_mode(dsi);
	msleep(150);

	mipi_dsi_dcs_write_seq(dsi, 0xb2, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0x53, 0x28);

	mipi_dsi_dcs_write_seq(dsi, 0xc7,
			       0xa7,0x53,0xff,0xd8,0xb2,
			       0x8c,0x5e,0x33,0x04);

	mipi_dsi_dcs_set_display_brightness(dsi, 0x00ff);

	msleep(50);

	return mipi_dsi_dcs_set_display_on(dsi);
}

static int wd5ea5f01_dsi_off(struct wd5ea5f01 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;

	mipi_dsi_dcs_set_display_off(dsi);
	msleep(50);

	mipi_dsi_dcs_enter_sleep_mode(dsi);
	msleep(120);

	return 0;
}

/* Power rails only */
static int wd5ea5f01_prepare(struct drm_panel *panel)
{
	struct wd5ea5f01 *ctx = to_wd5ea5f01(panel);
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_enable(ctx->vddio);
	if (ret) return ret;
	msleep(5);

	ret = regulator_enable(ctx->vci);
	if (ret) return ret;
	msleep(5);

	ret = regulator_enable(ctx->avdd);
	if (ret) return ret;
	msleep(10);

	ret = regulator_enable(ctx->elvdd);
	if (ret) return ret;
	msleep(10);

	wd5ea5f01_reset(ctx);
	msleep(20); // may need to bump this up to 30ms

	if (ctx->panel_enable_gpio)
		gpiod_set_value_cansleep(ctx->panel_enable_gpio, 1);
	msleep(10);

	ctx->prepared = true;
	return 0;
}

/* Send display ON */
static int wd5ea5f01_enable(struct drm_panel *panel)
{
	struct wd5ea5f01 *ctx = to_wd5ea5f01(panel);
	int ret;

	if (ctx->enabled)
		return 0;

	ret = wd5ea5f01_dsi_on(ctx);
	if (ret)
		return ret;

	ctx->enabled = true;
	return 0;
}

/* Send display OFF */
static int wd5ea5f01_disable(struct drm_panel *panel)
{
	struct wd5ea5f01 *ctx = to_wd5ea5f01(panel);

	if (!ctx->enabled)
		return 0;

	wd5ea5f01_dsi_off(ctx);
	ctx->enabled = false;
	return 0;
}

/* Remove power rails */
static int wd5ea5f01_unprepare(struct drm_panel *panel)
{
	struct wd5ea5f01 *ctx = to_wd5ea5f01(panel);

	if (!ctx->prepared)
		return 0;

	if (ctx->panel_enable_gpio)
		gpiod_set_value_cansleep(ctx->panel_enable_gpio, 0);
	msleep(20);

	regulator_disable(ctx->elvdd);
	msleep(10);

	regulator_disable(ctx->avdd);
	regulator_disable(ctx->vci);
	regulator_disable(ctx->vddio);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode wd5ea5f01_mode = {
	.clock = 150000,  // 150mHz pixel clock for safe PLL

	.hdisplay = 1080,
	.hsync_start = 1180,
	.hsync_end = 1188,
	.htotal = 1288,

	.vdisplay = 1920,
	.vsync_start = 1941,
	.vsync_end = 1945,
	.vtotal = 1952,

	.width_mm = 68,
	.height_mm = 121,

	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static int wd5ea5f01_get_modes(struct drm_panel *panel,
			      struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &wd5ea5f01_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);
	mode->type |= DRM_MODE_TYPE_PREFERRED;
	drm_mode_probed_add(connector, mode);

	connector->display_info.width_mm = 68;
	connector->display_info.height_mm = 121;

	return 1;
}

static const struct drm_panel_funcs wd5ea5f01_panel_funcs = {
	.prepare = wd5ea5f01_prepare,
	.enable = wd5ea5f01_enable,
	.disable = wd5ea5f01_disable,
	.unprepare = wd5ea5f01_unprepare,
	.get_modes = wd5ea5f01_get_modes,
};

static int wd5ea5f01_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct wd5ea5f01 *ctx;
	int ret;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	ctx->dsi = dsi;

	ctx->panel_enable_gpio =
		devm_gpiod_get(dev, "enable", GPIOD_OUT_LOW);
	if (IS_ERR(ctx->panel_enable_gpio))
		return PTR_ERR(ctx->panel_enable_gpio);

	ctx->reset_gpio =
		devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return PTR_ERR(ctx->reset_gpio);

	ctx->te_gpio = devm_gpiod_get_optional(dev, "te", GPIOD_IN);

	ctx->vddio = devm_regulator_get(dev, "vddio");
	if (IS_ERR(ctx->vddio)) return PTR_ERR(ctx->vddio);

	ctx->vci = devm_regulator_get(dev, "vci");
	if (IS_ERR(ctx->vci)) return PTR_ERR(ctx->vci);

	ctx->avdd = devm_regulator_get(dev, "avdd");
	if (IS_ERR(ctx->avdd)) return PTR_ERR(ctx->avdd);

	ctx->elvdd = devm_regulator_get(dev, "elvdd");
	if (IS_ERR(ctx->elvdd)) return PTR_ERR(ctx->elvdd);

	drm_panel_init(&ctx->panel, dev, &wd5ea5f01_panel_funcs,
		       DRM_MODE_CONNECTOR_DSI);
	drm_panel_add(&ctx->panel);

	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO |
			  MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_LPM;

	ret = mipi_dsi_attach(dsi);
	if (ret) {
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void wd5ea5f01_remove(struct mipi_dsi_device *dsi)
{
	struct wd5ea5f01 *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id wd5ea5f01_of_match[] = {
	{ .compatible = "boe,wd5ea5f01" },
	{ }
};
MODULE_DEVICE_TABLE(of, wd5ea5f01_of_match);

static struct mipi_dsi_driver wd5ea5f01_driver = {
	.probe = wd5ea5f01_probe,
	.remove = wd5ea5f01_remove,
	.driver = {
		.name = "panel-wd5ea5f01",
		.of_match_table = wd5ea5f01_of_match,
	},
};
module_mipi_dsi_driver(wd5ea5f01_driver);

MODULE_AUTHOR("Engineer162 <contact@hh-machinery.eu>");
MODULE_DESCRIPTION("DRM driver for BOE WD5EA5F01 AMOLED panel");
MODULE_LICENSE("GPL");