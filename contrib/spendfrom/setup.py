from distutils.core import setup
setup(name='abcspendfrom',
      version='1.0',
      description='Command-line utility for abcmint "coin control"',
      author='Gavin Andresen',
      author_email='gavin@abcmintfoundation.org',
      requires=['jsonrpc'],
      scripts=['spendfrom.py'],
      )
