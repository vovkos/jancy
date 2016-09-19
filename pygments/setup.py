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