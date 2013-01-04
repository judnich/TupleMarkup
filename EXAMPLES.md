## Tuple Markup Language Examples

_Note: This includes some redundant examples from the README, plus extra examples below._

### TML example demonstrating markup semantics:

    [html |
        Hello. This is an example [b|language] test.
    	[ div [class testc] | And this text is enclosed in a div. ]
    	[ a [href google.com] | Click this link [i|now] ]
    ]

Compare to HTML/XML:

    <html>
    	Hello. This is an example <b>language</b> test.
    	<div class='testc'> And this text is enclosed in a div. </div>
    	<a href='google.com'> Click this link <i>now</i> </a>
    </html>


### TML example demonstrating key-value pair semantics:

    [
    	[first name | John]
    	[last name | Smith]
    	[age | 25]
    	[address |
    		[street address | 21 2nd Street]
    		[city | New York]
    		[state | NY]
    		[postalCode | 10021]
    	]
    ]

Compare to JSON *(note - TML is NOT meant to replace JSON; this is only a syntax example)*:
    
    {
        "first name": "John",
        "last name": "Smith",
        "age": 25,
        "address": {
            "street address": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postalCode": 10021
        }
    }


### TML example describing a 3D pyramid object for OpenGL:

This example shows how one might use TML to load/store 3D models for an OpenGL graphics application. In addition to natural key-value pair syntax, TML enables concise lists of vertex coordinates and element indexes, making this very natural to read, write, and organize.

    [opengl model |
        [mode | indexed triangles]

        [buffer vertex |
            [attrib position | [layout interleaved] [type float3]]
            [attrib uv | [layout interleaved] [type float2]]

            [data position | [0 1 0] [-1 0 -1] [1 0 -1] [-1 0 1] [1 0 1]]
            [data uv | [0.5 0.0] [0 1] [1 1] [0 1] [1 1]]
        ]

        [buffer element16 |
            [data | 0 1 2  0 2 3  0 3 4  0 4 1 ]
        ]

        [texture 0 |
            [file | media/rock.png]
            [filter min | nearest]
            [filter mag | bilinear]
        ]

        [program fragment | media/rocky.frag]
        [program vertex | media/rocky.vert]
    ]


### TML example excerpt from a converted SVG image file:

This small part of a larger SVG (XML) file was converted (and formatted) to TML automatically using the included `tml-convert` commandline translation tool. SVG isn't particularly humanly readible, though TML is a bit better.

    [svg [version 1.1] [width 1000] [height 1100] [viewBox 0 0 1000 1100] [id svg3089] | 
        [defs [id defs3091] | 
            [linearGradient [id linearGradient3934] | 
                [stop [id stop3936] [style stop-color:#baa286;stop-opacity:1] [offset 0] |] 
                [stop [id stop3946] [style stop-color:#dccdba;stop-opacity:1] [offset 0.06] |] 
                [stop [id stop3942] [style stop-color:#dccdba;stop-opacity:1] [offset 0.73000002] |] 
                [stop [id stop3948] [style stop-color:#e2d7c8;stop-opacity:1] [offset 0.82999998] |] 
                [stop [id stop3944] [style stop-color:#c7ad95;stop-opacity:1] [offset 0.95999998] |] 
                [stop [id stop3938] [style stop-color:#c7ad95;stop-opacity:1] [offset 1] |] 
            ] 
            [linearGradient [id linearGradient3843] | 
                [stop [id stop3845] [style stop-color:#e3d6c3;stop-opacity:1] [offset 0] |] 
                [stop [id stop3847] [style stop-color:#d4bfa8;stop-opacity:1] [offset 0.34999999] |] 
                [stop [id stop3849] [style stop-color:#d4bfa8;stop-opacity:1] [offset 0.69999999] |] 
                [stop [id stop3851] [style stop-color:#e7ddc8;stop-opacity:1] [offset 1] |] 
            ]
            || ... this is just a small section from a large SVG file ...
        ]
    ]

Compare to XML:

    <?xml version="1.0" encoding="UTF-8" standalone="no"?>
    <svg xmlns:svg="http://www.w3.org/2000/svg" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink"
            version="1.1" width="1000" height="1100" viewBox="0 0 1000 1100" id="svg3089">
        <defs id="defs3091">
            <linearGradient id="linearGradient3934">
                <stop id="stop3936" style="stop-color:#baa286;stop-opacity:1" offset="0" />
                <stop id="stop3946" style="stop-color:#dccdba;stop-opacity:1" offset="0.06" />
                <stop id="stop3942" style="stop-color:#dccdba;stop-opacity:1" offset="0.73000002" />
                <stop id="stop3948" style="stop-color:#e2d7c8;stop-opacity:1" offset="0.82999998" />
                <stop id="stop3944" style="stop-color:#c7ad95;stop-opacity:1" offset="0.95999998" />
                <stop id="stop3938" style="stop-color:#c7ad95;stop-opacity:1" offset="1" />
            </linearGradient>
            <linearGradient id="linearGradient3843">
                <stop id="stop3845" style="stop-color:#e3d6c3;stop-opacity:1" offset="0" />
                <stop id="stop3847" style="stop-color:#d4bfa8;stop-opacity:1" offset="0.34999999" />
                <stop id="stop3849" style="stop-color:#d4bfa8;stop-opacity:1" offset="0.69999999" />
                <stop id="stop3851" style="stop-color:#e7ddc8;stop-opacity:1" offset="1" />
            </linearGradient>
            <!-- this is just a small section from a large SVG file -->
        </defs>
    </svg>
    
Parsing a large SVG file with Libxml2 for comparison showed:

* TML's default C/C++ parser is 5x faster than Libxml2
* TML's default C/C++ parser uses 1/5th the memory of Libxml2
* The TML file on disk is 10% smaller than the XML file


## License

Copyright (C) 2012 John Judnich. Released as open-source under The MIT Licence.

