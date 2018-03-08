create_clock -period 54Mhz -name clk54 [get_ports clock54]
create_clock -period 54Mhz -name _clk54 pll54|altpll_component|auto_generated|pll1|clk[0]
create_clock -period 27Mhz -name clk27 data:inst3|raw_counterX_reg[0]

create_clock -period 74.25Mhz -name clk74_175824 [get_ports clock74_175824]
create_clock -period 25.2MHz -name clk_hdmi pll74|altpll_component|auto_generated|pll1|clk[0]

set_clock_groups -exclusive -group _clk54 -group clk_hdmi
set_clock_groups -exclusive -group clk27 -group clk_hdmi

derive_pll_clocks
derive_clock_uncertainty