Tuple Markup Language
====

An extremely simple all-purpose markup language: nested lists with syntax that alleviates "bracket overload".
It enables JSON-like and XML-like semantics within the same clean and consistent language, plus much more.

TML Syntax
----

This is an example of a tuple of four items (a "4-tuple"):

    [a b c d]

You can nest tuples and data arbitrarily:

    [[a 1] c d [[a b c] [1 2 3]]]

Nesting tuples of tuples is a common case, so we provide two forms of special syntax for this.

The bar "|" delimiter creates a nested tuple out of each section it separates. For example:

    [ a b c | 1 2 3 | x | y | z ]

is equivalent to

    [ [a b c] [1 2 3] [x] [y] [z] ]

The "~" list delimiter creates nested tuples for each section only if the section contains more than one item:

    [ a b c ~ 1 2 3 ~ x ~ y ~ z ]

is equivalent to the following (note how the last three delimited sections are not tuple-ized):

    [ [a b c] [1 2 3] x y z ]

You can also write empty tuples if you wish: "[]". Empty tuples will also be generated with the "|" delimiter if you delimit nothingness: "[|]" is equivalent to "[[][]]". 

Line comments are supported. Simply prefix the comment with "~~". For example:

    ~~ This is a line comment example.

That's it! You now know 100% of TML.


Examples
----

A simple TML example:

    [
        [position [1 2 3]]
        [color blue]
        [scale [0.5 0.3]]
    ]

An equivalent TML code:

    [
    	[position ~ 1 2 3]
    	[color ~ blue]
    	[scale ~ 0.5 0.3]
    ]


TML example demonstrating markup semantics:

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


TML example demonstrating key-value pair semantics:

    [
    	[firstName ~ John]
    	[lastName ~ Smith]
    	[age ~ 25]
    	[address ~
    		[streetAddress ~ 21 2nd Street]
    		[city ~ New York]
    		[state ~ NY]
    		[postalCode ~ 10021]
    	]
    	[phoneNumber ~
    		[
    			[type ~ home]
    			[number ~ 212 555-1234]
    		]
    		[
    			[type ~ fax]
    			[number ~ 646 555-4567]
    		]
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
        },
        "phoneNumber": [
            {
                "type": "home",
                "number": "212 555-1234"
            },
            {
                "type": "fax",
                "number": "646 555-4567"
            }
        ]
    }
