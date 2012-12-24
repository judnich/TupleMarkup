#Tuple Markup Language Specification

### What is TML?

TML is an extremely simple all-purpose markup language: nested lists with bracket-minimizing syntax.
It enables XML-like and YAML-like semantics within the same clean and consistent language, plus much more.


### Line Comments

Warning: Keep in mind the EBNF grammar below does not take `||` line comments
into account. It's assumed that these will be removed as though by a preprocessor:
Any character sequence `||` (excepting `\||`) found within the code should be
ignored, plus every character up to and including the next line-feed character.
	

### EBNF Grammar Specification (Not Including Line Comments)

	syntax = grouped;

	grouped = (open, list, close) | (open, list, divider, list, close) ;
	list = {item} ;
	item = grouped | word ;

	open = {space}, "[", {space} ;
	close = {space}, "]", {space} ;
	divider = {space}, "|", {space} ;

	word = word-char, {word-char} ;
	word-char = "\s" | "\t" | "\r" | "\n" | "\[" | "\]" | "\|" | "\*" | "\?" | "\\"
	          | (char - space - "[" - "]" - "|" - "\") ;

	space = ? white space characters ? ;
	char = ? all visible characters ? ;


### Informal Notes

TML parsers generate nested lists of lists or word strings. In the EBNF grammar
above, only the "list" and "word" nodes make up the resulting TML tree.

Words are contiguous streams of arbitrary characters except whitespace and
the reserved symbols `[`, `|`, `]`, and `\` (escape code). Therefore, words are
separated by whitespace or by a reserved symbol (whitespace is not included in
parsed words). Whitespace can be added to words only through escape codes
`\s`, `\t`, etc.

