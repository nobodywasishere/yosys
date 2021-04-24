# Minimal makefile for Sphinx documentation
#

MAKEDIR := $(dir $(lastword $(MAKEFILE_LIST)))

# You can set these variables from the command line.
SPHINXOPTS      =
SPHINXBUILD     = [ -e env/bin/activate ] && source env/bin/activate; sphinx-build
SPHINXAUTOBUILD = [ -e env/bin/activate ] && source env/bin/activate; sphinx-autobuild
SPHINXPROJ      = SphinxContribHDLDiagramsDocs
SOURCEDIR       = .
BUILDDIR        = _build

# Put it first so that "make" without argument is like "make help".
help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

livehtml:
	@$(SPHINXAUTOBUILD) -b html --ignore \*.swp --ignore \*~ $(SPHINXOPTS) "$(SOURCEDIR)" "$(BUILDDIR)/html"

.PHONY: help livehtml Makefile

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
