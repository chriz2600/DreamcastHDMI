project_open DCxPlus

create_timing_netlist -model slow
read_sdc
update_timing_netlist

report_timing \
    -setup \
    -npaths 2000 \
    -detail full_path \
    -panel_name {Report Timing} \
    -multi_corner \
    -file "output_files/worst_case_paths.rpt"

project_close