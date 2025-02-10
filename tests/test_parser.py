import pytest
from sandbox import ReccmpTokenizer
from operator import itemgetter


def tk_values(tokens):
    return map(itemgetter(2), tokens)


@pytest.fixture(name="tk")
def tokenize_fn():
    return ReccmpTokenizer


def test_empty_file(tk):
    """No tokens for an empty file.
    It helps if you don't crash. Empty string is a null pointer."""
    assert [*tk("")] == []


def test_basic_string(tk):
    """Can read a string."""
    assert '"Test"' in tk_values(tk('"Test"'))


def test_empty_string(tk):
    """Can identify the empty string."""
    assert '""' in tk_values(tk('""'))


def test_string_escape_double(tk):
    """Ignore escaped double quote character."""
    assert '"Te\\"st"' in tk_values(tk('x = "Te\\"st"'))


def test_string_escape_slash(tk):
    assert '"Test\\\\"' in tk_values(tk('"Test\\\\" == test'))


def test_line_comment(tk):
    """Line comment should not include trailing LF"""
    assert "// test" in tk_values(tk("// test\n"))


def test_line_comment_eof(tk):
    """Should capture line comment even on the last line of the file"""
    assert "// test" in tk_values(tk("// test"))


def test_block_comment(tk):
    assert "/* test */" in tk_values(tk("/* test */"))


def test_block_comment_empty(tk):
    assert "/**/" in tk_values(tk("/**/"))


def test_block_comment_newlines(tk):
    code = "/* multi line\ncomment */"
    assert code in tk_values(tk(code))


def test_block_comment_one_star(tk):
    code = "/*/ test */"
    assert code in tk_values(tk(code))


