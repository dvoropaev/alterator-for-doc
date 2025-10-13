from setuptools import setup, find_packages

setup(
    name="alterator-entry",
    version="0.4.4",
    packages=find_packages(),
    install_requires=["jsonschema", "tomlkit"],
    python_requires=">=3.9",
)
