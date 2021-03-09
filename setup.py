from distutils.core import setup
import setuptools

setup(
    name='logtime',
    version='1.0.1',
    author='Matth Ingersoll',
    author_email='matth@mtingers.com',
    packages=['logtime',],
    license='BSD 2-Clause License',
    long_description=open('README.md').read(),
    long_description_content_type="text/markdown",
    url='https://github.com/mtingers/logtime',
    install_requires=[
    ],
    entry_points={
        'console_scripts': [
            'logtime=logtime.logtime:main',
        ],
    },
    # See https://pypi.python.org/pypi?%3Aaction=list_classifiers
    classifiers=[
        'Intended Audience :: System Administrators',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
    ],
)

