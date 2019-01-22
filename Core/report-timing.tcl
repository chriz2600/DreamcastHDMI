project_open DCxPlus

create_timing_netlist -model slow
read_sdc
update_timing_netlist

check_timing -include {latches loops}
report_timing \
    -setup \
    -npaths 2000 \
    -detail full_path \
    -panel_name {Report Timing} \
    -multi_corner \
    -file "output_files/worst_case_paths.rpt"

report_min_pulse_width \
    -nworst 2000 \
    -detail full_path \
    -panel_name {Report Timing} \
    -multi_corner \
    -file "output_files/worst_case_min_pulse_width.rpt"

project_close