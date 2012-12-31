## Tuple Markup Language

An extremely simple all-purpose markup language: nested lists with bracket-minimizing syntax.

### Why TML?

TML allows you to express human-readable data with a syntax much simpler and less cluttered than other all-purpose markup/data languages. TML reserves only four symbols, yet is flexible enough to express an extremely wide range of data semantics.

In contrast, many software engineers find XML to be overly verbose, irregular, complex, and bulky for most tasks. Related fact: The book "XML in a Nutshell" is **714** pages long.

* _"XML sometimes feels an awful lot like using an enormous sledgehammer to drive common household nails." - [Jeff Atwood](http://www.codinghorror.com/blog/2008/05/xml-the-angle-bracket-tax.html)_


### What is in this repository?

This repo contains TML parsers and document tree APIs implemented in a variety of languages -- currently C, C++, and JavaScript. Implementations in more languages (e.g. Go, Python, Ruby, etc.) are planned, as time permits. Refer to a TML parser implementation folder for examples and documentation on reading TML data in your application.


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


## TML Syntax

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

Empty tuples will also be generated with the `|` delimiter if you delimit nothingness: `[ | ]` is equivalent to `[ [] [] ]`. 


### Comments

Line comments are supported. Simply prefix the comment with `||`. For example:

    || This is a line comment example.

###Escape Codes

For special characters, you may use `\` for escape codes much like in C. Escape codes supported are:

    \n \r \t \s \\ \[ \] \| \? \*

These respectively evaluate to: `newline code, return code, tab code, space, \, [, ], |, code 0x01, code 0x02`.

The last two, `\?` and `\*`, are escape codes meant to be used as wildcard tokens by some TML APIs that allow you to use pattern-matching search queries on a TML tree (refer to the TML APIs documentation for more info). They don't have any unusual syntactic behavior.

### Done.

That's it! You now know all of TML. Take a look at the examples if you haven't already to see how it looks in use.


## License

Copyright (C) 2012 John Judnich. Released as open-source under The MIT Licence.

