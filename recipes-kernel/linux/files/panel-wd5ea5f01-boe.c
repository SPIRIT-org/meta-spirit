// SPDX-License-Identifier: GPL-2.0-only
// Copyright (c) 2026 FIXME
// Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
//   Copyright (c) 2013, The Linux Foundation. All rights reserved. (FIXME)

#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct wd5ea5f01_boe {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
};

static inline struct wd5ea5f01_boe *to_wd5ea5f01_boe(struct drm_panel *panel)
{
	return container_of(panel, struct wd5ea5f01_boe, panel);
}

static void wd5ea5f01_boe_reset(struct wd5ea5f01_boe *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(200);
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	msleep(200);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	msleep(200);
}

static int wd5ea5f01_boe_on(struct wd5ea5f01_boe *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf1, 0x5a, 0x5a);
	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 150);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb2, 0x03);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x28);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc7,
				     0xa7, 0x53, 0xff, 0xd8, 0xb2, 0x8c, 0x5e,
				     0x33, 0x04);
	mipi_dsi_dcs_set_display_brightness_multi(&dsi_ctx, 0x00ff);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 100);

	return dsi_ctx.accum_err;
}

static int wd5ea5f01_boe_off(struct wd5ea5f01_boe *ctx)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = ctx->dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 50);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static int wd5ea5f01_boe_prepare(struct drm_panel *panel)
{
	struct wd5ea5f01_boe *ctx = to_wd5ea5f01_boe(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	wd5ea5f01_boe_reset(ctx);

	ret = wd5ea5f01_boe_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		return ret;
	}

	return 0;
}

static int wd5ea5f01_boe_unprepare(struct drm_panel *panel)
{
	struct wd5ea5f01_boe *ctx = to_wd5ea5f01_boe(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	ret = wd5ea5f01_boe_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);

	return 0;
}

static const struct drm_display_mode wd5ea5f01_boe_mode = {
	.clock = (1080 + 100 + 8 + 100) * (1920 + 21 + 4 + 7) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 100,
	.hsync_end = 1080 + 100 + 8,
	.htotal = 1080 + 100 + 8 + 100,
	.vdisplay = 1920,
	.vsync_start = 1920 + 21,
	.vsync_end = 1920 + 21 + 4,
	.vtotal = 1920 + 21 + 4 + 7,
	.width_mm = 0,
	.height_mm = 0,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int wd5ea5f01_boe_get_modes(struct drm_panel *panel,
				   struct drm_connector *connector)
{
	return drm_connector_helper_get_modes_fixed(connector, &wd5ea5f01_boe_mode);
}

static const struct drm_panel_funcs wd5ea5f01_boe_panel_funcs = {
	.prepare = wd5ea5f01_boe_prepare,
	.unprepare = wd5ea5f01_boe_unprepare,
	.get_modes = wd5ea5f01_boe_get_modes,
};

static int wd5ea5f01_boe_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct wd5ea5f01_boe *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct wd5ea5f01_boe, panel,
				   &wd5ea5f01_boe_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST |
			  MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ret = drm_panel_of_backlight(&ctx->panel);
	if (ret)
		return dev_err_probe(dev, ret, "Failed to get backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "Failed to attach to DSI host\n");
	}

	return 0;
}

static void wd5ea5f01_boe_remove(struct mipi_dsi_device *dsi)
{
	struct wd5ea5f01_boe *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id wd5ea5f01_boe_of_match[] = {
	{ .compatible = "mdss,wd5ea5f01-boe" }, // FIXME
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, wd5ea5f01_boe_of_match);

static struct mipi_dsi_driver wd5ea5f01_boe_driver = {
	.probe = wd5ea5f01_boe_probe,
	.remove = wd5ea5f01_boe_remove,
	.driver = {
		.name = "panel-wd5ea5f01-boe",
		.of_match_table = wd5ea5f01_boe_of_match,
	},
};
module_mipi_dsi_driver(wd5ea5f01_boe_driver);

MODULE_AUTHOR("linux-mdss-dsi-panel-driver-generator <fix@me>"); // FIXME
MODULE_DESCRIPTION("DRM driver for wd5eaf01 boe 1080p video mode dsi panel");
MODULE_LICENSE("GPL");
