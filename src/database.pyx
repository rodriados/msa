#!/usr/bin/env python
# cython: language_level = 2
# Multiple Sequence Alignment sequence database wrapper file.
# @author Rodrigo Siqueira <rodriados@gmail.com>
# @copyright 2018-2019 Rodrigo Siqueira
from libcpp.string cimport string
from sequence cimport cSequence, Sequence
from database cimport cDatabase, cDatabaseEntry

# Represents a sequence stored in database.
# @since 0.1.1
class DatabaseEntry:

    # Initializes a new database entry representation.
    # @param str description The sequence description
    # @param Sequence sequence The sequence store in database
    def __init__(self, str description, str sequence):
        self.description = description
        self.sequence = Sequence(sequence)

    # Transforms the entry into the sequence for exhibition.
    # @return The database entry representation as a string.
    def __str__(self):
        return str(self.sequence)

# Stores a list of sequences read from possible different sources. The
# sequences may be identified by description or inclusion index.
# @since 0.1.1
cdef class Database:

    # Instantiates a new sequence database.
    # @param list args Positional arguments.
    def __cinit__(self, *args):
        self.add(*args)

    # Gives access to a specific sequence of the database.
    # @param int offset The requested sequence offset.
    # @return Sequence The requested sequence.
    def __getitem__(self, int offset):
        if offset >= self.count:
            raise IndexError("database index out of range")

        cdef cDatabaseEntry entry = self._ref.getEntry(offset)
        return DatabaseEntry(entry.description, entry.sequence.toString())

    # Adds new sequence(s) to database.
    # @param mixed arg The sequence(s) to add.
    def add(self, *args):
        for arg in args:
            if isinstance(arg, dict):
                self._addFromDict(arg)
            elif isinstance(arg, tuple):
                self._addFromTuple(arg[0], arg[1])
            elif isinstance(arg, str):
                self._addFromString(arg)
            elif isinstance(arg, Sequence):
                self._addFromSequence(arg)
            elif isinstance(arg, Database):
                self._addFromDatabase(arg)
            elif isinstance(arg, DatabaseEntry):
                self._addFromDatabaseEntry(arg)
            else:
                raise ValueError("Unknown sequence type.")

    @property
    # Informs the number of sequences in database.
    # @return int The number of sequences.
    def count(self):
        return self._ref.getCount()

    # Adds new database entries from a key-value dict.
    # @param arg The dict to be added to database.
    def _addFromDict(self, dict arg):
        for key, value in arg.iteritems():
            self._addFromTuple(str(key), value)

    # Adds new database entries from an already existing database.
    # @param dbase The database to be fused.
    def _addFromDatabase(self, Database dbase):
        for i in range(dbase.count):
            self._addFromDatabaseEntry(dbase[i])

    # Adds a new database entry from an already existing database entry.
    # @param entry The entry to be added to database.
    def _addFromDatabaseEntry(self, entry):
        self._addFromTuple(entry.description, str(entry.sequence))

    # Adds a new database entry from an already existing sequence.
    # @param sequence The sequence to be added to database.
    cdef void _addFromSequence(self, Sequence sequence):
        self._ref.add(sequence._ref)

    # Adds a new database entry from a sequence as string.
    # @param sequence The sequence to be added to database.
    cdef void _addFromString(self, string sequence):
        self._ref.add(cSequence(sequence))

    # Adds a new database entry from a entry tuple.
    # @param desc The sequence description.
    # @param sequence The sequence to be added to database.
    cdef void _addFromTuple(self, string desc, string sequence):
        self._ref.add(desc, cSequence(sequence))