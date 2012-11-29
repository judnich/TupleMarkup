#include "tml_parser.h"
#include "tml_tokenizer.h"

/*
 * Data is parsed and inserted into memory using a packed format to reduce the 
 * large overhead that would otherwise be incurred by the tree linkings (~24 bytes per node).
 * This format compresses that down to only 1 byte overhead per node for all sibling
 * leaf nodes.
 *
 * A node begins with the pointer data, followed by a null terminated value string IF
 * the pointer data indicates that this node is a leaf node.
 *
 * The pointer data is variable length, and has two forms:
 *
 * 1) One form is a single byte with value 0-253. In this case this value represents 
 * the relative offset to skip within the parsed data buffer to find the next subling node,
 * and also indicates that this is a leaf node (no child node pointer).
 *
 * 2) If the first byte is 255 then the next following bytes are a next_sibling
 * absolute offset followed by a first_child absolute offset.
 *
 * Because of the way the | nesting operator works (modifying the nesting depth of previously
 * parsed stuff), the parser needs to be able to insert an intermediate parent node sometimes.
 * To make this possible without shifting around memory, the parsing mechanism can just
 * enforce that all nodes following an open paren don't use the compressed form pointer data.
 */
 