/*
 * Copyright (C) 2012 John Judnich
 * Released as open-source under The MIT Licence.
 *
 * This simple utility converts XML to TML and vice versa.
 * 
 * Use it from the commandline by typing "tml-convert <source> <destination>"
 * where source is a tml or xml file you want to convert and destination is 
 * a filename of the opposite type you want to write the converted results to.
 */

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include "../../tml-c/source/tml_parser.h"

enum FILE_TYPE { FILE_XML, FILE_TML, FILE_UNKNOWN };

enum FILE_TYPE file_type(const char *filename)
{
	int len = strlen(filename);
	if (len < 4) return FILE_UNKNOWN;

	const char *ext = filename + len - 4;
	if (ext[0] == '.' && tolower(ext[1]) == 'x' && tolower(ext[2]) == 'm' && tolower(ext[3]) == 'l' ) return FILE_XML;
	if (ext[0] == '.' && tolower(ext[1]) == 't' && tolower(ext[2]) == 'm' && tolower(ext[3]) == 'l' ) return FILE_TML;
	return FILE_UNKNOWN;
}

int error(const char *msg)
{
	printf("\n(XML-TML Conversion Tool)\n");
	printf("%s\n\n", msg);
	return 1;
}

void write_tml_escaped_text(FILE *fout, const xmlChar *text)
{
	char ch = *text;
	bool spaced = false;

	while (ch) {
		if (ch == ' ') {
			if (spaced) fputs("\\s", fout);
		}
		else {
			if (spaced) fputc(' ', fout);

			if (ch == '\t') fputs("\\t\t", fout);
			else if (ch == '\n') fputs("\\n\n", fout);
			else if (ch == '\r') fputs("\\r", fout);
			else if (ch == '[') fputs("\\[", fout);
			else if (ch == ']') fputs("\\]", fout);
			else if (ch == '|') fputs("\\|", fout);
			else if (ch == '\\') fputs("\\\\", fout);
			else fputc(ch, fout);
		}

		spaced = (ch == ' ' || ch == '\t');
		ch = *text++;
	}
}

void write_tml_trimmed_text(FILE *fout, const xmlChar *text)
{
	bool wrote_word = false;
	char ch = *text;

	while (ch) {
		bool space = (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ');

		if (!space) {
			if (ch == '[') fputs("\\[", fout);
			else if (ch == ']') fputs("\\]", fout);
			else if (ch == '|') fputs("\\|", fout);
			else if (ch == '\\') fputs("\\\\", fout);
			else fputc(ch, fout);
			wrote_word = true;
		}
		else {
			if (wrote_word) fputc(' ', fout);
			wrote_word = false;
		}

		++text;
		ch = *text;
	}
}

void write_indented_newline(FILE *fout, int indent)
{
	fputc('\n', fout);
	for (int i = 0; i < indent; ++i) {
		fputc('\t', fout);
	}
}

void write_tml_node(FILE *fout, int indent, xmlNode *node)
{
	if (node->type == XML_ELEMENT_NODE) {
		write_indented_newline(fout, indent);

		// write [ name and attributes
		fputc('[', fout);
		write_tml_trimmed_text(fout, node->name);

		for (xmlAttr *attr = node->properties; attr; attr = attr->next) {
			fputs(" [", fout);
			write_tml_trimmed_text(fout, attr->name);
			fputc(' ', fout);
			write_tml_node(fout, 0, attr->children);
			fputc(']', fout);
		}

		// write contents
		fputs(" |", fout);
		if (node->children) {
			fputc(' ', fout);
			for (xmlNode *sub = node->children; sub; sub = sub->next) {
				write_tml_node(fout, indent+1, sub);
			}
		}

		// write ]
		if (node->children && node->children->next) {
			write_indented_newline(fout, indent);
		}
		fputs("] ", fout);
	}
	else if (node->type == XML_TEXT_NODE) {
		write_tml_trimmed_text(fout, node->content);
	}
	else if (node->type == XML_CDATA_SECTION_NODE) {
		write_tml_escaped_text(fout, node->content);
	}
}

void xml_to_tml(const char *source_file, const char *dest_file)
{   
	xmlDoc *doc = xmlReadFile(source_file, NULL, 0);

	if (doc == NULL) {
		exit(error("Error parsing XML file."));
	}

	xmlNode *root_node = xmlDocGetRootElement(doc);

	FILE *fout = fopen(dest_file, "w");
	if (!fout) {
		exit(error("Error writing to destination file."));
	}

	fputs("|| TML converted from XML\n", fout);
	write_tml_node(fout, 0, root_node);
	fputc('\n', fout);

	fclose(fout);

	xmlFreeDoc(doc);
	xmlCleanupParser();
}


void write_xml_escaped_text(FILE *fout, const char *text)
{
	char ch = *text;
	while (ch) {
		if (ch == ' ') fputs("&nbsp;", fout);
		else if (ch == '<') fputs("&lt;", fout);
		else if (ch == '>') fputs("&gt;", fout);
		else if (ch == '&') fputs("&amp;", fout);
		else if (ch == '\'') fputs("&apos;", fout);
		else if (ch == '\"') fputs("&quot;", fout);
		else fputc(ch, fout);
		++text;
		ch = *text;
	}
}

#define MAX_ATTRIB_SIZE 4096

void write_xml_attrib(FILE *fout, struct tml_node attrib)
{
	attrib = tml_first_child(&attrib);
	if (tml_is_null(&attrib)) return;

	write_xml_escaped_text(fout, attrib.value);
	fputs("=\"", fout);
	attrib = tml_next_sibling(&attrib);

	char buff[MAX_ATTRIB_SIZE];
	tml_node_to_string(&attrib, buff, MAX_ATTRIB_SIZE);
	write_xml_escaped_text(fout, buff);

	fputs("\"", fout);
}

static struct tml_node element_markup_pattern;

void write_xml_node(FILE *fout, int indent, struct tml_node node)
{
	if (tml_is_list(&node)) {
		// first make sure the node is of the expected format, e.g.:
		// [ element-name [attrib value] | element content ]
		if (tml_compare_nodes(&node, &element_markup_pattern)) {
			// iterate to the first meta and content items
			struct tml_node attrib, content, name_node;
			name_node = tml_first_child(&node);
			content = tml_next_sibling(&name_node);
			content = tml_first_child(&content);
			name_node = tml_first_child(&name_node);
			attrib = tml_next_sibling(&name_node);

			// write tag name
			write_indented_newline(fout, indent);
			fputc('<', fout);
			write_xml_escaped_text(fout, name_node.value);

			// write tag attributes
			while (!tml_is_null(&attrib)) {
				fputc(' ', fout);
				write_xml_attrib(fout, attrib);
				attrib = tml_next_sibling(&attrib);
			}

			// handle empty nodes
			if (tml_is_null(&content)) {
				fputs("/>", fout);
				return;
			}
			fputs(">", fout);

			// write the content nodes one by one
			bool multi_line = false;
			if (!tml_is_null(&content)) {
				while (!tml_is_null(&content)) {
					if (tml_is_list(&content)) multi_line = true;
					write_xml_node(fout, indent+1, content);
					fputc(' ', fout);
					content = tml_next_sibling(&content);
				}
			}
			
			// write closing tag
			if (multi_line)
				write_indented_newline(fout, indent);
			fputs("</", fout);
			write_xml_escaped_text(fout, name_node.value);
			fputc('>', fout);
		}
		else {
			// unexpected pattern, just recurse into its contents
			while (!tml_is_null(&node)) {
				write_xml_node(fout, indent, node);
				fputc(' ', fout);
				node = tml_next_sibling(&node);
			}
		}
	}
	else {
		write_xml_escaped_text(fout, node.value);
	}
}

void tml_to_xml(const char *source_file, const char *dest_file)
{
	struct tml_doc *pattern = tml_parse_string("[ \\? \\* | \\* ]");
	element_markup_pattern = pattern->root_node;

	struct tml_doc *doc = tml_parse_file(source_file);
	if (doc == NULL) {
		exit(error("Error parsing TML file."));
	}
	else if (doc->error_message) {
		exit(error(doc->error_message));
	}

	struct tml_node root_node = doc->root_node;

	FILE *fout = fopen(dest_file, "w");
	if (!fout) {
		exit(error("Error writing to destination file."));
	}

	fputs("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n", fout);
	fputs("<!--     XML converted from TML     -->", fout);
	write_xml_node(fout, 0, root_node);
	fputc('\n', fout);

	fclose(fout);

	tml_free_doc(doc);
	tml_free_doc(pattern);
}

void run_benchmark(const char *xml_file, const char *tml_file)
{
	int xml_time, tml_time;

	{
		clock_t t = clock();

		xmlDoc *doc = xmlReadFile(xml_file, NULL, 0);
		if (doc == NULL) exit(error("Error parsing XML file."));
		xmlFreeDoc(doc);
		xmlCleanupParser();

		xml_time = (int)(clock() - t);
	}

	{
		clock_t t = clock();

		struct tml_doc *doc = tml_parse_file(tml_file);
		tml_free_doc(doc);

		tml_time = (int)(clock() - t);
	}

	printf("Benchmarking default TML parser against Libxml2 XML parser...\n");
	printf("XML parse time: %d\n", xml_time);
	printf("TML parse time: %d\n", tml_time);

	double speedup = (double)xml_time / (double)tml_time;
	printf("Speedup: %f\n", speedup);
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		return error("\n\tUsage: tml_convert <source> <dest>");
	}

	bool benchmark = false;
	if (argc >= 4 && strcmp(argv[3], "--benchmark") == 0)
		benchmark = true;

	const char *source_file = argv[1];
	const char *dest_file = argv[2];

	enum FILE_TYPE source_type = file_type(source_file);
	enum FILE_TYPE dest_type = file_type(dest_file);

	if (source_type == FILE_UNKNOWN)
		return error("Unknown source file type.");
	if (dest_type == FILE_UNKNOWN)
		return error("Unknown destination file type.");
	if (source_type == dest_type)
		return error("Source and destination are both the same file type - nothing to convert.");

	if (!benchmark) {
		if (source_type == FILE_XML) {
			xml_to_tml(source_file, dest_file);
		}
		else {
			tml_to_xml(source_file, dest_file);
		}
	}
	else {
		const char *tml_file, *xml_file;
		if (source_type == FILE_XML) {
			xml_file = source_file;
			tml_file = dest_file;
		}
		else {
			tml_file = source_file;
			xml_file = dest_file;
		}
		run_benchmark(xml_file, tml_file);
	}
	
	return 0;
}
