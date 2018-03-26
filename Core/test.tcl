# Define a few helper procedures to print out points
# on a path, and the path itself
proc get_clock_string { path clk } {
    set clk_str ""
    set clk_id [ get_path_info $path -${clk}_clock ]
    if { $clk_id ne "" } {
        set clk_str [ get_clock_info $clk_id -name ]
        if { [ get_path_info $path -${clk}_clock_is_inverted ] } {
            append clk_str " (INVERTED)"
        }
    }
    return $clk_str
}
proc print_point { point } {
    set total [ get_point_info $point -total ]
    set incr [ get_point_info $point -incr ]
    set node_id [ get_point_info $point -node ]
    set type [ get_point_info $point -type ]
    set rf [ get_point_info $point -rise_fall]
    set node_name ""
    if { $node_id ne "" } {
        set node_name [ get_node_info $node_id -name ]
    }
    puts [format "%10s %8s %2s %-6s %s" $total $incr $rf $type $node_name ]
}
proc print_path { path } {
    puts "Slack : [ get_path_info $path -slack]"
    puts "To Clock : [ get_clock_string $path to ]"
    puts "From Clock : [ get_clock_string $path from]"
    puts ""
    puts [format "%10s %8s %-2s %-6s %s" "Total" "Incr" "RF" "Type" "Name"]
    puts "=================================================================="
    foreach_in_collection pt [ get_path_info $path -arrival_points ] {
        print_point $pt
    }
}
project_open DCxPlus-10CL025
# Always create the netlist first
create_timing_netlist
read_sdc DCxPlus.sdc
update_timing_netlist
# And now simply iterate over the 10 worst setup paths, printing each
# path
foreach_in_collection path [ get_timing_paths -npaths 2 -setup ] {
    print_path $path
    puts ""
}
report_clock_fmax_summary
report_net_timing -nworst_delay 2 -stdout hans
delete_timing_netlist
project_close