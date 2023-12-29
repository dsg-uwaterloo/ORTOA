from setuptools import find_namespace_packages, setup

setup(
    name="ortoa-sdk",
    packages=find_namespace_packages(include=["extras.ortoa.*"]),
)
