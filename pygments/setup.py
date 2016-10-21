#...............................................................................
#
#  This file is part of the Jancy toolkit.
#
#  Jancy is distributed under the MIT license.
#  For details see accompanying license.txt file,
#  the public copy of which is also available at:
#  http://tibbo.com/downloads/archive/jancy/license.txt
#
#...............................................................................

from setuptools import setup, find_packages

setup (
    name='jancylexer',
    packages=find_packages(),
    entry_points =
    """
    [pygments.lexers]
    jancylexer = jancylexer.lexer:JancyLexer
    """,
)
