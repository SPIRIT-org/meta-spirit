# meta-plasma expects old split package names
# Fix for "Nothing RPROVIDES 'libcanberra-pulse' (but /work/build/../meta-kde/recipes-plasma/plasma-pa_6.5.2.bb RDEPENDS on or otherwise requires it)"

RPROVIDES:${PN} += " \
    libcanberra-pulse \
    libcanberra-alsa \
"

RREPLACES:${PN} += " \
    libcanberra-pulse \
    libcanberra-alsa \
"

RCONFLICTS:${PN} += " \
    libcanberra-pulse \
    libcanberra-alsa \
"
