
#pragma once

#include <iostream>

#include <string>

namespace gismo {

/** 
    \brief This class is used to create a Paraview .pvd (collection)
    file. 

    A collection is an XML file that contains a list of other
    files to be opened in Paraview.
    
    Typical usage is
    \verbatim
    gsParaviewCollection pc(fn); // Initialize collection
    
    // add files ("parts"), make sure that these files exist
    pc.addPart(filename1);
    pc.addPart(filename2);

    pc.save() // finalize and close the file
    \endverbatim

    The above creates a file with extension pvd. When opening this
    file with Paraview, the containts of all parts in the list are
    loaded.
        
*/
class gsParaviewCollection
{
public:
    typedef std::string String;
public:
    
    /// Constructor using a filename.
    gsParaviewCollection(std::string const & fn)
        : counter(0)
    {
        String mfn(fn);
        mfn.append(".pvd");
        mfile = new std::ofstream( mfn.c_str() );
        GISMO_ASSERT(mfile->is_open(), "Error creating "<< mfn );

        *mfile <<"<?xml version=\"1.0\"?>\n";
        *mfile <<"<VTKFile type=\"Collection\" version=\"0.1\">";
        *mfile <<"<Collection>\n";
    }

    /// Destructor: save() should be called before reaching here
    ~gsParaviewCollection()
    {
        GISMO_ASSERT(!mfile->is_open(), 
                     "gsParaviewCollection Error: Call save() before exit." );
        delete mfile;
    }

    /// Adds a part in the collection, with complete filename (including extension) \a fn
    void addPart(String const & fn)
    {
        GISMO_ASSERT(fn.find_last_of(".") != String::npos, "File without extension");
        GISMO_ASSERT(mfile->is_open(), "Error: "<< mfile <<" is not open." );
        *mfile << "<DataSet part=\""<<counter++<<"\" file=\""<<fn<<"\"/>\n";
    }

    /// Adds a part in the collection, with filename \a fn with extension \a ext appended
    void addPart(String const & fn, String const & ext)
    {
        GISMO_ASSERT(mfile->is_open(), "Error: "<< mfile <<" is not open." );
        *mfile << "<DataSet part=\""<<counter++<<"\" file=\""<<fn<<ext<<"\"/>\n";
    }

    /// Adds a part in the collection, with filename \a fn_i and extension \a ext appended
    void addPart(String const & fn, int i, String const & ext)
    {
        GISMO_ASSERT(mfile->is_open(), "Error: "<< mfile <<" is not open." );
        *mfile << "<DataSet part=\""<<i<<"\" file=\""<<fn<<"_"<<i<<ext<<"\"/>\n";
    }
    
    // to do: make time collections as well
    //void addTimestep(String const & fn)
    //{
    //    *mfile << "<DataSet timestep=\""<<i<<"\" file=\""<<fn<<"\"/>\n";
    //}

    /// Finalizes the collection by closing the XML tags, always call
    /// this function (once) when you finish adding files
    void save()
    {
        GISMO_ASSERT(mfile->is_open(), "Error: "<< mfile <<" is not open." );
        *mfile <<"</Collection>\n";
        *mfile <<"</VTKFile>\n";
        mfile->close();
    }

private:
    /// Pointer to the open file
    std::ofstream * mfile;

    /// Counter for the number of parts (files) added in the collection
    int counter;

private:
    // Construction without a filename is not allowed
    gsParaviewCollection();
};

/// Fast creation of a collection using base filename \a fn, extension
/// \a ext.  The collection will contain the files fn_0.ext,
/// fn_1.ext,...,fn_{n-1}.ext In the special case of n=0, the
/// collection is just fn.pvd and contains only the file fn.ext
inline void makeCollection(std::string const & fn, std::string const & ext, int n = 0)
{
    gsParaviewCollection pc(fn);
    if ( n > 0)
    {
        for (int i=0; i<n ; i++)
            pc.addPart(fn, i, ext);
    }
    else
        pc.addPart(fn, ext);

    pc.save();
}


}; // end namespace gismo
