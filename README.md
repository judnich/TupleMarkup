#Tuple Markup Language

An extremely simple all-purpose markup language: nested lists with syntax that alleviates "bracket overload".
It enables JSON-like and XML-like semantics within the same clean and consistent language, plus much more.

### What is in this repository?

This repo contains TML parsers and query APIs implemented in a variety of languages. These allow your applications to quickly and easily access data from TML files. Refer to a TML parser implementation folder for documentation on how to get started using TML files in your application language of choice.

##TML Syntax

###Basic Tuples

This is an example of a tuple of four items (a "4-tuple"):

    [a b c d]

You can nest tuples and data arbitrarily, for example:

    [ [a b c] 1 2 3 [[x][y]] ]

Writing empty tuples is also valid: "[]". 

Nesting tuples of tuples is a common case, so we provide two forms of special syntax for this.

###Nesting Delimiter

The bar "|" delimiter creates a nested tuple out of each section it separates. For example:

    [ position | 1 2 3 ]

...is equivalent to:

    [ [position] [1 2 3] ]

Empty tuples will also be generated with the "|" delimiter if you delimit nothingness: "[ | ]" is equivalent to "[ [] [] ]". 

###Nonsingleton Nesting Delimiter

The "~" delimiter creates nested tuples for each section only if the section contains more than one item:

    [ position ~ 1 2 3 ]

...is equivalent to the following (note how the left section, containing only one item "position", is not nested into its own tuple):

    [ position [1 2 3] ]

This is quite useful for uncluttered name-value pairs, for example.

###Comments

Line comments are supported. Simply prefix the comment with "~~". For example:

    ~~ This is a line comment example.

###Done.

That's it! You now know all of TML. Take a look at the examples below to see how it looks in use.


##Examples

###A simple TML example:

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


###TML example demonstrating markup semantics:

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


###TML example demonstrating key-value pair semantics:

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
