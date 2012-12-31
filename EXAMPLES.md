# TML Tutorial and Examples


## What is TML?

TML allows you to express human-readable data with a syntax much simpler and less cluttered than other all-purpose markup/data languages. TML reserves only four symbols, yet is flexible enough to express an extremely wide range of data semantics.

In contrast, many software engineers find XML to be overly verbose, irregular, complex, and bulky for most tasks. Related fact: The book "XML in a Nutshell" is **714** pages long.

* _"XML sometimes feels an awful lot like using an enormous sledgehammer to drive common household nails." - [Jeff Atwood](http://www.codinghorror.com/blog/2008/05/xml-the-angle-bracket-tax.html)_



## TML Syntax Tutorial

### Basic Tuples

This is an example of a tuple of four items (a "4-tuple"):

    [a b c d]

You can nest tuples and data arbitrarily, for example:

    [ [blah blah] 1 2 3 [[x][y]] ]

Writing empty tuples is also valid: `[]`. Nesting tuples of tuples is a common case, so we provide a special syntax for this.

### Nesting Delimiter

The bar `|` delimiter creates a nested tuple out of each section it separates. For example:

    [ position | 1 2 3 ]

is equivalent to:

    [ [position] [1 2 3] ]

Keep in mind you can use it as many times as you like, for example:

    [abc|def|ghi]

is equivalent to:

    [[abc] [def] [ghi]]

Empty tuples will also be generated with the `|` delimiter if you delimit nothingness: `[ | ]` is equivalent to `[ [] [] ]`.


### Comments

Line comments are supported. Simply prefix the comment with `||`. For example:

    || This is a line comment example.

### Escape Codes

For special characters, you may use `\` for escape codes much like in C. Escape codes supported are:

    \n \r \t \s \\ \[ \] \| \? \*

These respectively evaluate to: `newline code, return code, tab code, space, \, [, ], |, code 0x01, code 0x02`.

The last two, `\?` and `\*`, are escape codes meant to be used as wildcard tokens by some TML APIs that allow you to use pattern-matching search queries on a TML tree (refer to the TML APIs documentation for more info). They don't have any unusual syntactic behavior.

### Done.

That's it! You now know all of TML. Take a look at the examples if you haven't already to see how it looks in use.



## TML Examples

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
    	[firstName | John]
    	[lastName | Smith]
    	[age | 25]
    	[address |
    		[streetAddress | 21 2nd Street]
    		[city | New York]
    		[state | NY]
    		[postalCode | 10021]
    	]
    ]

Compare to JSON:
    
    {
        "firstName": "John",
        "lastName": "Smith",
        "age": 25,
        "address": {
            "streetAddress": "21 2nd Street",
            "city": "New York",
            "state": "NY",
            "postalCode": 10021
        }
    }

### TML example describing a 3D pyramid object for OpenGL:

    [opengl model |
        [mode | indexed triangles]

        [vbo |
            [type | vertex]

            [attrib position | [offset 0] [stride 20] [type float3]]
            [attrib uv | [offset 12] [stride 20] [type float2]]

            [buffer position | [0 1 0] [-1 0 -1] [1 0 -1] [-1 0 1] [1 0 1]]
            [buffer uv | [0.5 0.0] [0 1] [1 1] [0 1] [1 1]]
        ]

        [vbo |
            [type | element16]
            [buffer | 0 1 2  0 2 3  0 3 4  0 4 1 ]
        ]

        [texture 0 |
            [file | media/rock.png]
            [filter min | nearest]
            [filter mag | bilinear]
        ]

        [program fragment | media/rocky.frag]
        [program vertex | media/rocky.vert]
    ]

### TML example excerpt from a converted SVG Image file:

This small part of a larger SVG (XML) file was converted (and formatted) to TML automatically using the included tml-convert translation tool. SVG isn't particularly humanly readible, even in TML (though not as bad as XML).

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
            [linearGradient [id linearGradient3825] | 
                [stop [id stop3827] [style stop-color:#ede5d5;stop-opacity:1] [offset 0] |] 
                [stop [id stop3837] [style stop-color:#dcceba;stop-opacity:1] [offset 0.34999999] |] 
                [stop [id stop3835] [style stop-color:#e0cfb7;stop-opacity:1] [offset 0.69999999] |] 
                [stop [id stop3829] [style stop-color:#e7ddc8;stop-opacity:1] [offset 1] |] 
            ] 
            [linearGradient [id linearGradient3719] | 
                [stop [id stop3721] [style stop-color:#d4bfa8;stop-opacity:1] [offset 0] |] 
                [stop [id stop3723] [style stop-color:#d4bfa8;stop-opacity:1] [offset 0.72000003] |] 
                [stop [id stop3725] [style stop-color:#ede5d5;stop-opacity:1] [offset 1] |] 
            ] 
            || ... this is just a small section from a large SVG file ...
        ]
    ]


## License

Copyright (C) 2012 John Judnich. Released as open-source under The MIT Licence.

