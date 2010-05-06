#ifndef PUGIXML_NO_XPATH

#include "common.hpp"

#if defined(_MSC_VER) && _MSC_VER == 1200
#	define MSVC6_NAN_BUG // NaN comparison on MSVC6 is incorrect, see http://www.nabble.com/assertDoubleEquals,-NaN---Microsoft-Visual-Studio-6-td9137859.html
#endif

#if defined(__INTEL_COMPILER) && __INTEL_COMPILER == 800
#	define MSVC6_NAN_BUG // IC8 seems to have the same bug as MSVC6 does
#endif

#if defined(__BORLANDC__)
#	define MSVC6_NAN_BUG // BCC seems to have the same bug as MSVC6 does
#endif

TEST_XML(xpath_operators_arithmetic, "<node><foo-bar>10</foo-bar><foo>2</foo><bar>3</bar></node>")
{
	xml_node c;
	xml_node n = doc.child(STR("node"));

	// incorrect unary operator
	CHECK_XPATH_FAIL(STR("-"));

	// correct unary operator
	CHECK_XPATH_NUMBER(c, STR("-1"), -1);
	CHECK_XPATH_NUMBER(c, STR("--1"), 1);
	CHECK_XPATH_NUMBER(c, STR("---1"), -1);

	// incorrect binary operators
	CHECK_XPATH_FAIL(STR("5+"));
	CHECK_XPATH_FAIL(STR("5-"));
	CHECK_XPATH_FAIL(STR("5*"));
	CHECK_XPATH_FAIL(STR("+5"));
	CHECK_XPATH_FAIL(STR("*5"));
	CHECK_XPATH_FAIL(STR("1div2"));
	CHECK_XPATH_FAIL(STR("1mod"));
	CHECK_XPATH_FAIL(STR("1div"));

	// correct trivial binary operators
	CHECK_XPATH_NUMBER(c, STR("1 + 2"), 3);
	CHECK_XPATH_NUMBER(c, STR("1+2"), 3);
	CHECK_XPATH_NUMBER(c, STR("1 * 2"), 2);
	CHECK_XPATH_NUMBER(c, STR("1*2"), 2);
	CHECK_XPATH_NUMBER(c, STR("1 div 2"), 0.5);

	// operator precedence
	CHECK_XPATH_NUMBER(c, STR("2 + 2 * 2 div 1 mod 3"), 3);
	CHECK_XPATH_NUMBER(c, STR("2 + 2 * 2 div (1 mod 3)"), 6);
	CHECK_XPATH_NUMBER(c, STR("(2 + 2) * 2 div (1 mod 3)"), 8);
	CHECK_XPATH_NUMBER(c, STR("(2 + 2) * (2 div 1) mod 3"), 2);
	CHECK_XPATH_NUMBER(c, STR("2 - -2"), 4);
	CHECK_XPATH_NUMBER(c, STR("2--2"), 4);
	CHECK_XPATH_NUMBER(c, STR("1-2-3"), -4);

	// infinity/nan
	CHECK_XPATH_STRING(c, STR("1 div 0"), STR("Infinity"));
	CHECK_XPATH_STRING(c, STR("-1 div 0"), STR("-Infinity"));
	CHECK_XPATH_STRING(c, STR("-1 div 0 + 1 div 0"), STR("NaN"));
	CHECK_XPATH_STRING(c, STR("0 div 0"), STR("NaN"));
	CHECK_XPATH_STRING(c, STR("1 div 0 + 1 div 0"), STR("Infinity"));
	CHECK_XPATH_STRING(c, STR("-1 div 0 + -1 div 0"), STR("-Infinity"));
	CHECK_XPATH_STRING(c, STR("1 div 0 + 100"), STR("Infinity"));
	CHECK_XPATH_STRING(c, STR("-1 div 0 + 100"), STR("-Infinity"));
	CHECK_XPATH_STRING(c, STR("0 div 0 + 100"), STR("NaN"));

	// mod, from W3C standard
	CHECK_XPATH_NUMBER(c, STR("5 mod 2"), 1);
	CHECK_XPATH_NUMBER(c, STR("5 mod -2"), 1);
	CHECK_XPATH_NUMBER(c, STR("-5 mod 2"), -1);
	CHECK_XPATH_NUMBER(c, STR("-5 mod -2"), -1);

	// correct subtraction parsing, from W3C standard
	CHECK_XPATH_NUMBER(n, STR("foo-bar"), 10);
	CHECK_XPATH_NUMBER(n, STR("foo -bar"), -1);
	CHECK_XPATH_NUMBER(n, STR("foo - bar"), -1);
	CHECK_XPATH_NUMBER(n, STR("-foo-bar"), -10);
	CHECK_XPATH_NUMBER(n, STR("-foo -bar"), -5);
}

TEST(xpath_operators_logical)
{
	xml_node c;

	// boolean arithmetic
	CHECK_XPATH_BOOLEAN(c, STR("true() or true()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("true() or false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("false() or false()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("false() or true()"), true);

	CHECK_XPATH_BOOLEAN(c, STR("true() and true()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("true() and false()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("false() and false()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("false() and true()"), false);
	
	// boolean conversion
	CHECK_XPATH_BOOLEAN(c, STR("1 or ''"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 and ''"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 or ''"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 or 'a'"), true);
}

TEST(xpath_operators_equality_primitive_boolean)
{
	xml_node c;

	// boolean vs boolan
	CHECK_XPATH_BOOLEAN(c, STR("true() = true()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("false() = false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("true() != false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("false() != false()"), false);

	// upcast to boolean
	CHECK_XPATH_BOOLEAN(c, STR("true() = 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("true() != 2"), false);
	CHECK_XPATH_BOOLEAN(c, STR("false() = 2"), false);
	CHECK_XPATH_BOOLEAN(c, STR("false() != 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("false() = 0"), true);
	CHECK_XPATH_BOOLEAN(c, STR("false() != 0"), false);

	CHECK_XPATH_BOOLEAN(c, STR("2 = true()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("2 != true()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("2 = false()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("2 != false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("0 = false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("0 != false()"), false);
}

TEST(xpath_operators_equality_primitive_number)
{
	xml_node c;

	// number vs number
	CHECK_XPATH_BOOLEAN(c, STR("1 = 1"), true);
	CHECK_XPATH_BOOLEAN(c, STR("0.5 = 0.5"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 != 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 = -1"), false);

	// infinity/nan
	CHECK_XPATH_BOOLEAN(c, STR("1 div 0 = 2 div 0"), true);
	CHECK_XPATH_BOOLEAN(c, STR("-1 div 0 != 2 div 0"), true);

#ifndef MSVC6_NAN_BUG
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 = 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 != 1"), true);
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 = 0 div 0"), false);
#endif

	// upcast to number
	CHECK_XPATH_BOOLEAN(c, STR("2 = '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("2 != '2'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("'1' != 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("'1' = 2"), false);
}

TEST(xpath_operators_equality_primitive_string)
{
	xml_node c;

	// string vs string
	CHECK_XPATH_BOOLEAN(c, STR("'a' = 'a'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("'a' = 'b'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("'ab' != 'a'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("'' != 'a'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("'a' != ''"), true);
	CHECK_XPATH_BOOLEAN(c, STR("'' != ''"), false);
}

TEST_XML(xpath_operators_equality_node_set_node_set, "<node><c1><v>a</v><v>b</v></c1><c2><v>a</v><v>c</v></c2><c3><v>b</v></c3><c4><v>d</v></c4><c5><v>a</v><v>b</v></c5><c6><v>b</v></c6></node>")
{
	xml_node c;
	xml_node n = doc.child(STR("node"));

	// node set vs node set
	CHECK_XPATH_BOOLEAN(c, STR("x = x"), false); // empty node set compares as false with any other object via any comparison operator, as per XPath spec
	CHECK_XPATH_BOOLEAN(c, STR("x != x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = c2/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c2/v = c3/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = c4/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x = c1"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v != c2/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c2/v != c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != c4/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != c5/v"), true); // (a, b) != (a, b), since a != b, as per XPath spec (comparison operators are so not intutive)
	CHECK_XPATH_BOOLEAN(n, STR("c3/v != c6/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x != c1/v"), false);
}

TEST_XML(xpath_operators_equality_node_set_primitive, "<node><c1><v>1</v><v>-1</v><v>100</v></c1><c2><v>1</v><v>nan</v></c2></node>")
{
	xml_node c;
	xml_node n = doc.child(STR("node"));

	// node set vs number
	CHECK_XPATH_BOOLEAN(c, STR("x = 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x != 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 = x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 != x"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v = 1"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = -1"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != 1"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = 5"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c2/v = 1"), true);

	CHECK_XPATH_BOOLEAN(n, STR("1 = c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("-1 = c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("1 != c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("5 = c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("1 = c2/v"), true);

#ifndef MSVC6_NAN_BUG
	CHECK_XPATH_BOOLEAN(n, STR("c2/v != 1"), true);
	CHECK_XPATH_BOOLEAN(n, STR("1 != c2/v"), true);
#endif
	
	// node set vs string
	CHECK_XPATH_BOOLEAN(c, STR("x = '1'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x != '1'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("'1' = x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("'1' != x"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v = '1'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = '-1'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != '1'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = '5'"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c2/v = '1'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c2/v != '1'"), true);

	CHECK_XPATH_BOOLEAN(n, STR("'1' = c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'-1' = c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'1' != c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'5' = c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("'1' = c2/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'1' != c2/v"), true);

	// node set vs almost-numeric string just in case
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = '1.0'"), false);

	// node set vs boolean - special rules! empty sets are equal to true()
	CHECK_XPATH_BOOLEAN(n, STR("x = true()"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x != true()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("x = false()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = true()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v != true()"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v = false()"), false);

	CHECK_XPATH_BOOLEAN(n, STR("true() = x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("true() != x"), true);
	CHECK_XPATH_BOOLEAN(n, STR("false() = x"), true);
	CHECK_XPATH_BOOLEAN(n, STR("true() = c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("true() != c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("false() = c1/v"), false);
}

TEST(xpath_operators_inequality_primitive)
{
	xml_node c;

	// number vs number
	CHECK_XPATH_BOOLEAN(c, STR("1 < 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 <= 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 > 2"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 >= 2"), false);

	CHECK_XPATH_BOOLEAN(c, STR("1 < 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 <= 1"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 > 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 >= 1"), true);

	// infinity/nan
	CHECK_XPATH_BOOLEAN(c, STR("1 div 0 <= 2 div 0"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 div 0 < 2 div 0"), false);
	CHECK_XPATH_BOOLEAN(c, STR("-1 div 0 < 2 div 0"), true);
	CHECK_XPATH_BOOLEAN(c, STR("-1 div 0 > 2 div 0"), false);

#ifndef MSVC6_NAN_BUG
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 < 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 <= 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 > 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 div 0 >= 1"), false);
#endif

	// upcast to number
	CHECK_XPATH_BOOLEAN(c, STR("2 < '2'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 < '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("2 <= '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("3 <= '2'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("2 > '2'"), false);
	CHECK_XPATH_BOOLEAN(c, STR("3 > '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("2 >= '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("3 >= '2'"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 >= true()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 > true()"), false);
}

TEST_XML(xpath_operators_inequality_node_set_node_set, "<node><c1><v>1</v><v>-1</v><v>-100</v></c1><c2><v>1</v><v>nan</v></c2><c3><v>1</v><v>-4</v></c3></node>")
{
	xml_node c;
	xml_node n = doc.child(STR("node"));

	// node set vs node set
	CHECK_XPATH_BOOLEAN(c, STR("x < x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x > x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x <= x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x >= x"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v > x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= x"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= x"), false);

	CHECK_XPATH_BOOLEAN(n, STR("x > c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x < c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x >= c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("x <= c1/v"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v > c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < c3/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= c3/v"), true);

#ifndef MSVC6_NAN_BUG
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > c2/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= c2/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < c2/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= c2/v"), true);
#endif
}

TEST_XML(xpath_operators_inequality_node_set_primitive, "<node><c1><v>1</v><v>-1</v><v>-100</v></c1><c2><v>1</v><v>nan</v></c2></node>")
{
	xml_node c;
	xml_node n = doc.child(STR("node"));

	// node set vs number
	CHECK_XPATH_BOOLEAN(c, STR("x < 0"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x > 0"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x <= 0"), false);
	CHECK_XPATH_BOOLEAN(c, STR("x >= 0"), false);

	CHECK_XPATH_BOOLEAN(c, STR("0 < x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 > x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 <= x"), false);
	CHECK_XPATH_BOOLEAN(c, STR("0 >= x"), false);

	CHECK_XPATH_BOOLEAN(n, STR("c1/v > 0"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > 1"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= 0"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < 0"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= 0"), true);

	CHECK_XPATH_BOOLEAN(n, STR("0 < c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("1 < c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("0 <= c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("0 > c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("0 >= c1/v"), true);

	// node set vs string
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > '0'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > '1'"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= '0'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < '0'"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= '0'"), true);

	CHECK_XPATH_BOOLEAN(n, STR("'0' < c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'1' < c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("'0' <= c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'0' > c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("'0' >= c1/v"), true);

	// node set vs boolean
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > false()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v > true()"), false);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v >= false()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v < false()"), true);
	CHECK_XPATH_BOOLEAN(n, STR("c1/v <= false()"), true);

	CHECK_XPATH_BOOLEAN(n, STR("false() < c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("true() < c1/v"), false);
	CHECK_XPATH_BOOLEAN(n, STR("false() <= c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("false() > c1/v"), true);
	CHECK_XPATH_BOOLEAN(n, STR("false() >= c1/v"), true);
}

TEST(xpath_operators_boolean_precedence)
{
	xml_node c;

	CHECK_XPATH_BOOLEAN(c, STR("1 = 0 or 2 = 2"), true);
	CHECK_XPATH_BOOLEAN(c, STR("1 = (0 or 2) = false()"), false);
	CHECK_XPATH_BOOLEAN(c, STR("1 < 0 or 2 > 2"), false);
	CHECK_XPATH_BOOLEAN(c, STR("2 < 1 = false()"), true);
	CHECK_XPATH_BOOLEAN(c, STR("2 < (1 = false())"), false);
	CHECK_XPATH_BOOLEAN(c, STR("3 > 2 > 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("(3 > 2) > 1"), false);
	CHECK_XPATH_BOOLEAN(c, STR("3 > (2 > 1)"), true);
}

TEST_XML(xpath_operators_union, "<node><employee/><employee secretary=''/><employee assistant=''/><employee secretary='' assistant=''/><employee assistant='' secretary=''/><tail/></node>")
{
	doc.precompute_document_order();

	xml_node c;
	xml_node n = doc.child(STR("node"));

	CHECK_XPATH_NODESET(n, STR("employee | .")) % 2 % 3 % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR("employee[@secretary] | employee[@assistant]")) % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR("employee[@assistant] | employee[@secretary]")) % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR("employee[@secretary] | employee[@nobody]")) % 4 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR("employee[@nobody] | employee[@secretary]")) % 4 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR("tail/preceding-sibling::employee | .")) % 2 % 3 % 4 % 6 % 8 % 11;
	CHECK_XPATH_NODESET(n, STR(". | tail/preceding-sibling::employee | .")) % 2 % 3 % 4 % 6 % 8 % 11;
}

#endif
