set debug_mode 0 ;# debugdevice printing mode
set pos        0
set addr       0
set ppos       0

proc pause_on {} {
    #ext debugdevice
    set use_pause true
    debug set_watchpoint write_io {0x2e} {} {process_ctrl $::wp_last_value}
}

proc pause_off {} {
    #ext debugdevice
    set use_pause false
    debug set_watchpoint write_io {0x2e} {} {}
}

proc process_ctrl {{value 0}} {
    global use_pause
    global debug_mode
    switch $value {
        255 {
            if {$use_pause > 0} {
                debug break
            }
        }
        default { set debug_mode $value }
    }
}

proc addr2string {addr} {
    set str ""
    for {set byte [peek $addr]} {$byte > 0} {incr addr; set byte [peek $addr]} {
        append str [format "%c" $byte]
    }
    return $str
}

# formatting commands
proc printf__c   {mod addr} { puts -nonewline stderr [format "%${mod}c"  [peek [peek16 $addr]]] }
proc printf__s   {mod addr} { puts -nonewline stderr [format "%${mod}s"  [addr2string [peek16 $addr]]] }
proc printf__S   {mod addr} { puts -nonewline stderr [format "%${mod}s"  [string toupper  [addr2string [peek16 $addr]]]] }
proc printf__hhi {mod addr} { puts -nonewline stderr [format "%${mod}hi" [peek8  [peek16 $addr]]] }
proc printf__hi  {mod addr} { puts -nonewline stderr [format "%${mod}hi" [peek16 [peek16 $addr]]] }
proc printf__i   {mod addr} { printf__hi $mod $addr }
proc printf__hhu {mod addr} { puts -nonewline stderr [format "%${mod}hu" [peek8  [peek16 $addr]]] }
proc printf__hu  {mod addr} { puts -nonewline stderr [format "%${mod}hu" [peek16 [peek16 $addr]]] }
proc printf__u   {mod addr} { printf__hu $mod $addr }
proc printf__hhx {mod addr} { puts -nonewline stderr [format "%${mod}hx" [peek8  [peek16 $addr]]] }
proc printf__hx  {mod addr} { puts -nonewline stderr [format "%${mod}hx" [peek16 [peek16 $addr]]] }
proc printf__x   {mod addr} { printf__hx $mod $addr }
proc printf__hhX {mod addr} { puts -nonewline stderr [format "%${mod}hX" [peek8  [peek16 $addr]]] }
proc printf__hX  {mod addr} { puts -nonewline stderr [format "%${mod}hX" [peek16 [peek16 $addr]]] }
proc printf__X   {mod addr} { printf__hX $mod $addr }
proc printf__hho {mod addr} { puts -nonewline stderr [format "%${mod}ho" [peek8  [peek16 $addr]]] }
proc printf__ho  {mod addr} { puts -nonewline stderr [format "%${mod}ho" [peek16 [peek16 $addr]]] }
proc printf__o   {mod addr} { printf__ho $mod $addr }
proc printf__hhb {mod addr} { puts -nonewline stderr [format "%${mod}hb" [peek8  [peek16 $addr]]] }
proc printf__hb  {mod addr} { puts -nonewline stderr [format "%${mod}hb" [peek16 [peek16 $addr]]] }
proc printf__b   {mod addr} { printf__hb $mod $addr }
proc printf__f   {mod addr} { puts -nonewline stderr [format "%${mod}s"  [parse_basic_float 3 [peek16 $addr]]] }
proc printf__lf  {mod addr} { puts -nonewline stderr [format "%${mod}s"  [parse_basic_float 7 [peek16 $addr]]] }
proc printf__hf  {mod addr} { puts -nonewline stderr [format "%${mod}s"  [parse_sdcc_float  [peek16 $addr]]] }
#proc printf__hhf {mod addr} { puts -nonewline stderr [format "%${mod}s"  [parse_fp_float    [peek16 $addr]]] }
proc printf__?       {addr} { puts -nonewline stderr [format "%s"        [print_debug_mode    [peek16 [peek16 $addr]]]] }
proc printf__z   {mod addr} { puts stderr "mod=$mod" } ;# debug

# MSX-BASIC single is 1-bit signal + 7-bit exponent + (3|7) bytes packed BCD (n*2 digits)
proc parse_basic_float {size addr} {
    set buf "0."
    set tmp [peek8 $addr]
    set signal [expr $tmp & 0x80 ? {"-"} : {"+"}]
    set exponent [expr ($tmp & 0x7f) - 0x40]
    set mantissa [debug read_block memory [expr $addr + 1] $size] ;# read_block returns a string
    for {set b 0} {$b < $size} {incr b} {
        set i [scan [string index $mantissa $b] %c]
        append buf [format %x $i]
    }
    append buf "e$signal$exponent"
    return $buf
}

# format is unknown (even searching in the manual)
proc parse_sdcc_float {val} {
    append buf "??.??"
}

# compatibility with old debugging code
proc print_debug_mode {value} {
    global debug_mode
    switch $debug_mode {
        0 { return [format %x $value] }
        1 { return [format %i $value] }
        2 { return [format %b $value] }
        3 { return [format %c [expr $value & 0xff]] }
        default { return "" }
    }
}

# empty lots of variables at once
proc empty-> {args} {
    for {set len 0} {$len < [llength $args]} {incr len} {
        upvar [lindex $args $len] arg; set arg ""
    }
}

proc printf {addr} {
    global ppos
    set fmt_addr [peek16 $addr]
    set ending_addr $fmt_addr
    set arg_addr [expr $addr + 2]
    set neg   ""  ;# negative sign?
    set lpad  ""  ;# pad size in characters
    set tdot  ""  ;# truncated dot?
    set rpad  ""  ;# truncated size in characters
    set cats  ""  ;# category suffix
    set raw   ""

    for {set byte [peek $ending_addr]} {$byte > 0} {incr ending_addr; set byte [peek $ending_addr]} {
        set c [format %c $byte]
        switch $c {
            "%" { if {$ppos eq 1} { set ppos 0; append raw $c } else { incr ppos } }
            "c" { if {$ppos > 0}  { set ppos 0; set cmd "printf__c        {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "S" { if {$ppos > 0}  { set ppos 0; set cmd "printf__S        {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "s" { if {$ppos > 0}  { set ppos 0; set cmd "printf__s        {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "i" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}i {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "d" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}i {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "u" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}u {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "x" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}x {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "X" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}X {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "o" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}o {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "b" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}b {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "f" { if {$ppos > 0}  { set ppos 0; set cmd "printf__${cats}f {$neg$lpad$tdot$rpad} $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "?" { if {$ppos > 0}  { set ppos 0; set cmd "printf__? $arg_addr"; empty-> neg lpad tdot rpad cats; incr arg_addr 2 } else { append raw $c } }
            "h" { if {$ppos > 0}  { append cats $c; incr ppos } else { append raw $c } }
            "l" { if {$ppos > 0}  { append cats $c; incr ppos } else { append raw $c } }
            default {
                if {$ppos > 0} {
                    if {$c eq "-"} {
                        append neg $c
                    } elseif {$ppos > 0 && $c eq "."} {
                        append tdot $c
                    } elseif {$ppos > 0 && $byte >= 48 && $byte <= 57} {
                        if {$tdot eq ""} { append lpad $c } else { append rpad $c }
                    }
                    incr ppos
                } else {
                    ;# fall through
                    set ppos 0; append raw $c
                }
            }
        }
        if {$ppos eq 0} {
            if {[info exists cmd]} {
                puts -nonewline stderr $raw; set raw ""
                eval $cmd
                unset cmd
            } else {
                puts -nonewline stderr $raw; set raw ""
            }
        }
    }
}

proc debug_printf {value} {
    global pos
    global addr

    if {$pos == 1} {
        set addr [expr ($value << 8) + $addr]
        printf $addr
        set addr 0
        incr pos -1
    } else {
        set addr $value
        incr pos
    }
}

if { [info exists ::env(DEBUG)] && $::env(DEBUG) > 0 } {
    set use_pause $::env(DEBUG)
    #ext debugdevice
    debug set_watchpoint write_io {0x2e} {} {process_ctrl $::wp_last_value}
    debug set_watchpoint write_io {0x2f} {} {debug_printf $::wp_last_value}
}

ext debugdevice
