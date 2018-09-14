/**
 * Multiple Sequence Alignment fasta file.
 * @author Rodrigo Siqueira <rodriados@gmail.com>
 * @copyright 2018 Rodrigo Siqueira
 */
#include <fstream>
#include <cstring>
#include <string>

#include "msa.hpp"
#include "fasta.hpp"
#include "cluster.hpp"

/**
 * Instantiates a new fasta file sequence list.
 * @param fname The name of file to be opened and extracted.
 */
Fasta::Fasta(const std::string& fname)
{
    onlymaster {
        this->load(fname);
        debug("loaded %lu sequences from %s", this->getCount(), fname.c_str());
    }

    broadcast(*this);
}

/**
 * Reads a file and allocates memory to all sequences contained in it.
 * @param fname The name of the file to be loaded.
 */
void Fasta::load(const std::string& fname)
{
    std::fstream ffile(fname, std::fstream::in);

    if(ffile.fail())
        finalize(Error("input file is invalid or does not exist."));

    while(!ffile.eof() && !ffile.fail())
        this->extract(ffile);

    ffile.close();
}

/**
 * Extracts a sequence out of the file and puts it into a buffer.
 * @param ffile The file to read sequence from.
 * @param dest The destination address for the sequence.
 * @return Could a sequence be extracted?
 */
bool Fasta::extract(std::fstream& ffile)
{
    std::string buffer, description, sequence;

    while(!ffile.eof() && ffile.peek() != 0x3E)
        // Ignore all characters until a '>' is reached.
        // Our sequence will always have a description.
        ffile.get();

    if(ffile.eof())
        return false;

    std::getline(ffile, description);
    description.erase(0, 1);

    while(ffile.peek() != 0x3E && std::getline(ffile, buffer) && buffer.size() > 0)
        sequence.append(buffer);

    this->push(description, sequence);

    return true;
}

/**
 * Pushes a new sequence into the list.
 * @param description The new sequence description.
 * @param string A string that will originate a new sequence into the list.
 */
void Fasta::push(const std::string& description, const std::string& sequence)
{
    this->list.push_back(FastaSequence(description, sequence));
}

/**
 * Pushes a new sequence into the list.
 * @param description The new sequence description.
 * @param buffer The buffer that will originate a new sequence into the list.
 * @param size The buffer's size.
 */
void Fasta::push(const std::string& description, const char *buffer, size_t size)
{
    this->list.push_back(FastaSequence(description, buffer, size));
}

/**
 * Sends the sequences loaded by the master node to all other nodes.
 * This method will send all sequences to all nodes.
 * @param fasta The target instance for broadcast.
 */
void broadcast(Fasta& fasta)
{
    size_t count = fasta.getCount();
    cluster::broadcast(&count);
    cluster::sync();

    size_t *sizes = new size_t[count];
    size_t szsum = 0;

    onlymaster {
        for(size_t i = 0; i < count; ++i)
            szsum += sizes[i] = fasta[i].getLength();
    }

    cluster::broadcast(sizes, count);
    cluster::broadcast(&szsum);
    cluster::sync();

    char *data = new char[szsum];

    onlymaster {
        for(size_t i = 0, offset = 0; i < count; ++i) {
            memcpy(&data[offset], fasta[i].getBuffer(), sizeof(char) * sizes[i]);
            offset += sizes[i];
        }
    }

    cluster::broadcast(data, szsum);
    cluster::sync();

    onlyslaves {
        for(size_t i = 0, offset = 0; i < count; ++i) {
            fasta.push("__slave", data + offset, sizes[i]);
            offset += sizes[i];
        }
    }

    delete[] sizes;
    delete[] data;
}