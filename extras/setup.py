from setuptools import setup, find_namespace_packages

setup(
    name="ortoa-sdk",
    packages=find_namespace_packages(include=["extras.ortoa.*"]),
)
