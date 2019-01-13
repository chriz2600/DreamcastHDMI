create_clock -name virtual54 -period 54Mhz

# input clock
create_clock -period 54Mhz -waveform { 4.629 13.888 } -name clk54 [get_ports clock54]
create_generated_clock -name datain_clock -source {pll54|altpll_component|auto_generated|pll1|inclk[0]} {pll54|altpll_component|auto_generated|pll1|clk[0]}

# output clocks
create_clock -period 74.25Mhz -name clk74_175824 [get_ports clock74_175824]
create_generated_clock -name data_clock -source {pll_hdmi|altpll_component|auto_generated|pll1|inclk[0]} {pll_hdmi|altpll_component|auto_generated|pll1|clk[0]}
create_generated_clock -name clock_clock -source {pll_hdmi|altpll_component|auto_generated|pll1|inclk[0]} -phase 0 {pll_hdmi|altpll_component|auto_generated|pll1|clk[1]}
create_generated_clock -name output_clock -source {pll_hdmi|altpll_component|auto_generated|pll1|clk[1]} [get_ports CLOCK]

##################
# internal clock #
##################
#create_clock -name int_osc_clk -period 80MHz {control_clock_gen|int_osc_0|wire_sd1_clkout}
#create_generated_clock -name pll_reconfig_clock -source {control_clock_gen|int_osc_0|wire_sd1_clkout} -divide_by 2 -multiply_by 1 "control_clock_2"
#create_clock -name int_osc_clk -period 80MHz "control_clock"

set_false_path -from [get_ports {HDMI_INT_N}]
set_false_path -from [get_ports {video_mode_480p_n}]

#derive_pll_clocks -create_base_clocks

set_clock_groups -asynchronous -group datain_clock -group data_clock
set_clock_groups -asynchronous -group datain_clock -group clock_clock
set_clock_groups -asynchronous -group datain_clock -group output_clock
#set_clock_groups -asynchronous -group datain_clock -group int_osc_clk
#set_clock_groups -asynchronous -group data_clock -group int_osc_clk

derive_clock_uncertainty

# input delays
set tSU 2.0 # orig: 1.3
set tH 2.0  # orig: 1.0
set dcinputs [get_ports {data* _hsync _vsync}]
set_input_delay -max -clock virtual54 $tSU $dcinputs
set_input_delay -min -clock virtual54 -$tH $dcinputs
set_false_path -setup -rise_from [get_clocks virtual54] -fall_to [get_clocks datain_clock]
set_false_path -setup -fall_from [get_clocks virtual54] -rise_to [get_clocks datain_clock]
set_false_path -hold -rise_from [get_clocks virtual54] -rise_to [get_clocks datain_clock]
set_false_path -hold -fall_from [get_clocks virtual54] -fall_to [get_clocks datain_clock]

# output delays
set tSU 2.0 # orig: 1.3, adv ds: 1.0
set tH 2.0  # orig: 1.0, adv ds: 0.7
set adv_clock_delay 0.0
set hdmi_outputs [get_ports {VIDEO* DE HSYNC VSYNC}]
set_output_delay -clock output_clock -reference_pin [get_ports CLOCK] -max [expr $tSU - $adv_clock_delay] $hdmi_outputs
set_output_delay -clock output_clock -reference_pin [get_ports CLOCK] -min [expr 0 - $tH - $adv_clock_delay ] $hdmi_outputs
set_false_path -setup -rise_from [get_clocks data_clock] -fall_to [get_clocks output_clock]
set_false_path -setup -fall_from [get_clocks data_clock] -rise_to [get_clocks output_clock]
set_false_path -hold -rise_from [get_clocks data_clock] -rise_to [get_clocks output_clock]
set_false_path -hold -fall_from [get_clocks data_clock] -fall_to [get_clocks output_clock]
set_false_path -to [remove_from_collection [all_outputs] "$hdmi_outputs"]
