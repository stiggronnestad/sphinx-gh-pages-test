FROM python:3.10-slim

# Install system packages needed for doxygen, graphviz and building docs
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    doxygen \
    graphviz \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# Upgrade pip and setuptools
RUN pip install --no-cache-dir --upgrade pip setuptools wheel

# Install Python packages: Sphinx, Breathe, and optional themes or extensions
RUN pip install --no-cache-dir \
    sphinx \
    breathe \
    sphinx-rtd-theme \
    myst-parser

# If you rely on Doxygen configuration via doxypypy (optional):
# RUN pip install doxypypy

# Set up a working directory for your documentation
WORKDIR /src

# By default, this container will run a shell, but you can override the entrypoint/CMD
# to build docs. For example (uncomment if desired):
CMD ["bash"]
