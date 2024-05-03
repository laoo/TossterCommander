        data

        align   8

tos_image::
        rept    32768
        dc.q    $544f53494d414745
        endr
tos_version::
        rept    32
        dc.b    0
        endr
