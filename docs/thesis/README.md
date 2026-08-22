# Thesis LaTeX Sources

**Temporal Accuracy in Rehosted Firmware — A Temporal Fidelity Layer for the
Renode Emulation Framework** (MSc thesis, Saarland University).

## Layout

```
docs/thesis/
├── main.tex                  # root document
├── references.bib            # bibliography (BibTeX)
├── frontmatter/              # title page, declaration, abstract, acknowledgements
├── chapters/                 # chapters 1–7
├── appendix/                 # appendices A (hardware) and B (reproducibility)
└── figures/                  # evaluation figures (regenerate: python analysis/eval_full.py)
```

## Building

### Overleaf (recommended)
Zip this `docs/thesis/` folder, upload it as a new Overleaf project, and set
`main.tex` as the root document. Compiler: **pdfLaTeX**.

### Local
```bash
cd docs/thesis
pdflatex main
bibtex   main
pdflatex main
pdflatex main
# or simply:
latexmk -pdf main.tex
```

Requires a full TeX Live installation (packages: siunitx, booktabs, tikz,
listings, natbib, cleveref, fancyhdr, subcaption).

## Regenerating figures

The six evaluation figures in `figures/` are produced by the analysis
pipeline in the repository root:

```bash
pip install -r analysis/requirements.txt
python analysis/eval_full.py
cp analysis/figures/*.pdf docs/thesis/figures/
```

## Things to fill in before submission

- Supervisor and reviewer names on the title page
  (`frontmatter/titlepage.tex`) and in the acknowledgements.
- Submission date (title page uses `\today` by default).
- Check your department's current declaration wording
  (`frontmatter/declaration.tex`).
