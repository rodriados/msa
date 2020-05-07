#!/usr/bin/env python
# Multiple Sequence Alignment parser wrapper file.
# @author Rodrigo Siqueira <rodriados@gmail.com>
# @copyright 2018-2019 Rodrigo Siqueira
from libcpp.vector cimport vector
from libcpp.string cimport string
from database cimport c_database, Database
from parser cimport *

# Parses a list of files and produces a list of database entries.
# @param list filenames The list of files to parse.
# @param str extension Parse all files using this parser.
# @return The database containing all parsed sequences.
def parse(list filenames, **kwargs):
    cdef vector[string] files = [filename.encode() for filename in filenames]
    cdef string extension = kwargs.pop('extension', str()).encode()
    cdef c_database contents = c_parse(files, extension) if extension.size() else c_parse(files)
    return Database.wrap(contents)

# Parses a FASTA file.
# @param str filename The file to be parsed.
# @return list List containing all entries parsed from file.
def fasta(list filenames):
    cdef c_database result
    [result.merge(c_fasta(filename.encode())) for filename in filenames]
    return Database.wrap(result)
