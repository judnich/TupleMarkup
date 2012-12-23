
var TML = (function() {
	var me = {};

	me.parse = function(tmlString)
	{
		var tokens = tmlString.match(/(\[|\]|\||\w+)/g);
		return parseRoot(tokens);
	}

	me.toMarkupString = function(tmlTree)
	{
		return _treeToString(tmlTree, true);
	}

	me.toString = function(tmlTree)
	{
		return _treeToString(tmlTree, false);
	}


	// Parses list "... ]" where the open bracket has already been read.
	// If endAtBar is true, this will let "|" terminate the list.
	var parseList = function(tokens, index, endAtBar)
	{
		var i = index;

		var tokLen = tokens.length;
		var list = [];
		while (i < tokLen) {
			tok = tokens[i];
			++i;

			if (tok === "[") {
				// append sublist
				var ret = parseList(tokens, i);
				i = ret.index;
				list.push(ret.result);
			}
			else if (tok === "|") {
				if (endAtBar) 
					return {index:i, result:list};

				// nest-ify
				list = [list];
				while (i < tokLen) {
					var ret = parseList(tokens, i, true);
					i = ret.index;

					list.push(ret.result);

					if (tokens[i-1] === "]")
						return {index:i, result:list};
				}
				break; // expected closing bracket, reached EOF
			}
			else if (tok === "]") {
				// close this list
				return {index:i, result:list};
			}
			else {
				// append word
				if (tok !== "") list.push(tok);
			}
		}

		throw "TML: Expected close bracket";	
	}

	var parseRoot = function(tokens)
	{
		var tokLen = tokens.length;
		if (tokLen == 0) return null;

		var tok = tokens[0];
		if  (tok !== "[") throw "TML: Expected open bracket";

		var ret = parseList(tokens, 1, false);
		var tree = ret.result;

		if (ret.index < tokLen) throw "TML: Expected end of file after root node";

		return tree;
	}


	var _treeToString = function(tmlTree, brackets)
	{
		var len = tmlTree.length;
		var str = "";

		if (brackets) str = "[";

		for (var i = 0; i < len; ++i) {
			var v = tmlTree[i];

			if (typeof(v) === "string")
				str += v;
			else
				str += _treeToString(v, brackets);

			if (i < len-1) str += " ";
		}

		if (brackets) str += "]";

		return str;
	}

	return me;
}());



