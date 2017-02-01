set_global_assignment -entity "pll_tx_cyclone4_vga" -library "lib_pll_tx_cyclone4_vga" -name IP_TOOL_NAME "altera_pll"
set_global_assignment -entity "pll_tx_cyclone4_vga" -library "lib_pll_tx_cyclone4_vga" -name IP_TOOL_VERSION "16.0"
set_global_assignment -entity "pll_tx_cyclone4_vga" -library "lib_pll_tx_cyclone4_vga" -name IP_TOOL_ENV "mwpim"
set_global_assignment -library "lib_pll_tx_cyclone4_vga" -name SPD_FILE [file join $::quartus(sip_path) "pll_tx_cyclone4_vga.spd"]

set_global_assignment -library "lib_pll_tx_cyclone4_vga" -name MISC_FILE [file join $::quartus(sip_path) "pll_tx_cyclone4_vga_sim/pll_tx_cyclone4_vga.vo"]
