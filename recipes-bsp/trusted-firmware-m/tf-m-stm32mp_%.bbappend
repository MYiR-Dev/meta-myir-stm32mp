# Remove interrupt-controller binding from the PMIC YAML for MYD-LD25X M33-TD.
# The M33 (TDCID=OFF) does not have RIF access to EXTI1, causing IAC
# exception ID 153. The PMIC interrupt is only used by CA35.
SRC_URI:append:myd-ld25x-m33td = " file://0001-stpmic-yaml-no-irq.patch"
