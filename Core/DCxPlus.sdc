create_clock -period 54Mhz -name clk54 [get_ports clock54]
create_clock -period 74.25Mhz -name clk74_175824 [get_ports clock74_175824]
create_clock -period 27Mhz -name clk27 {data:video_input|raw_counterX_reg[0]}
set_false_path -from [get_ports {HDMI_INT_N}]
set_false_path -from [get_ports {video_mode_480p_n}]

derive_pll_clocks -create_base_clocks

set_clock_groups -exclusive \
    -group pll54|altpll_component|auto_generated|pll1|clk[0] \
    -group pll74|altpll_component|auto_generated|pll1|clk[0]
set_clock_groups -exclusive \
    -group clk27 \
    -group pll74|altpll_component|auto_generated|pll1|clk[0]

derive_clock_uncertainty

# input delay
set tSU 1.0
set tH 0.7
set dcinputs [get_ports {data* _hsync _vsync}]
set_input_delay -clock pll54|altpll_component|auto_generated|pll1|clk[0] -clock_fall -max $tSU $dcinputs -add_delay
set_input_delay -clock pll54|altpll_component|auto_generated|pll1|clk[0] -clock_fall -min -$tH $dcinputs -add_delay

#set_input_delay -clock pll74|altpll_component|auto_generated|pll1|clk[0] 0 [get_ports {SDAT SCLK}]

# output delays
set tSU 1.0
set tH 0.7
set adv_clock_delay 0
set hdmi_outputs [get_ports {VIDEO* DE HSYNC VSYNC}]
set_output_delay -clock {pll74|altpll_component|auto_generated|pll1|clk[0]} -reference_pin [get_ports CLOCK] -max [expr $tSU - $adv_clock_delay] $hdmi_outputs
set_output_delay -clock {pll74|altpll_component|auto_generated|pll1|clk[0]} -reference_pin [get_ports CLOCK] -min [expr 0 - $tH - $adv_clock_delay ] $hdmi_outputs
set_false_path -to [remove_from_collection [all_outputs] "$hdmi_outputs"]
