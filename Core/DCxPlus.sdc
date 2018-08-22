create_clock -name virtual54 -period 54Mhz

create_clock -period 74.25Mhz -name clk74_175824 [get_ports clock74_175824]
create_clock -period 54Mhz -name clk54 [get_ports clock54]

create_generated_clock -name data_clock -source {pll54|altpll_component|auto_generated|pll1|inclk[0]} -phase 180 {pll54|altpll_component|auto_generated|pll1|clk[0]}
#create_generated_clock -name output_clock -source {pll_hdmi|altpll_component|auto_generated|pll1|inclk[0]} {pll_hdmi|altpll_component|auto_generated|pll1|clk[0]}

set_false_path -from [get_ports {HDMI_INT_N}]
set_false_path -from [get_ports {video_mode_480p_n}]

derive_pll_clocks -create_base_clocks

set output_clock "pll_hdmi|altpll_component|auto_generated|pll1|clk[0]"


set_clock_groups -exclusive -group data_clock -group $output_clock

derive_clock_uncertainty

# input delay
set tSU 1.0
set tH 0.7
set dcinputs [get_ports {data* _hsync _vsync}]
#set_input_delay -clock virtual54 -clock_fall -max $tSU $dcinputs -add_delay
#set_input_delay -clock virtual54 -clock_fall -min -$tH $dcinputs -add_delay
set_input_delay -max -clock virtual54 $tSU $dcinputs
set_input_delay -min -clock virtual54 -$tH $dcinputs
#set_input_delay -max -clock virtual54 -clock_fall $tSU $dcinputs -add
#set_input_delay -min -clock virtual54 -clock_fall -$tH $dcinputs -add
set_false_path -setup -rise_from [get_clocks virtual54] -fall_to [get_clocks data_clock]
set_false_path -setup -fall_from [get_clocks virtual54] -rise_to [get_clocks data_clock]
set_false_path -hold -rise_from [get_clocks virtual54] -rise_to [get_clocks data_clock]
set_false_path -hold -fall_from [get_clocks virtual54] -fall_to [get_clocks data_clock]

# output delays
set tSU 1.0
set tH 0.7
set adv_clock_delay 0
set hdmi_outputs [get_ports {VIDEO* DE HSYNC VSYNC}]
set_output_delay -clock $output_clock -reference_pin [get_ports CLOCK] -max [expr $tSU - $adv_clock_delay] $hdmi_outputs
set_output_delay -clock $output_clock -reference_pin [get_ports CLOCK] -min [expr 0 - $tH - $adv_clock_delay ] $hdmi_outputs
set_false_path -to [remove_from_collection [all_outputs] "$hdmi_outputs"]
