# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'evert-firmware'
copyright = '2024, Evert AS'
author = 'Evert AS'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
    'myst_parser'
]

templates_path = ['_templates']
exclude_patterns = []

breathe_projects = {
    "doxygen-boost-converter": "../doxygen-boost-converter/xml",
    "doxygen-ccu": "../doxygen-ccu/xml",
    "doxygen-core": "../doxygen-core/xml",
    "doxygen-dcdc": "../doxygen-dcdc/xml",
    "doxygen-device": "../doxygen-device/xml",
    "doxygen-inverter": "../doxygen-inverter/xml",
}
breathe_default_project = "doxygen-inverter"
breathe_implementation_filename_extensions = ['.c', '.cc', '.cpp']
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

