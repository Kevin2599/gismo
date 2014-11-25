
#pragma once

#include <gsCore/gsDomainIterator.h>

#include <gsUtils/gsCombinatorics.h>

#include <gsAssembler/gsGaussRule.h>

namespace gismo
{
// Documentation in gsDomainIterator.h
// Class which enables iteration over all elements of a tensor product parameter domain

/** Re-implements gsDomainIterator for iteration over all elements of a <b>tensor product</b> parameter domain.\n
 * <em>See gsDomainIterator for more detailed documentation and an example of the typical use!!!</em>
 */

template<class T, int D>
class gsTensorDomainIterator : public gsDomainIterator<T>
{
private:
    typedef typename std::vector<T>::const_iterator  uiter;

public:

    // note: this assumes that b is a tensor product basis
    gsTensorDomainIterator(const gsBasis<T>& b)
        : gsDomainIterator<T>( b ),
          d( m_basis.dim() ),
          lower ( gsVector<T, D>::Zero(d) ),
          upper ( gsVector<T, D>::Zero(d) ) 
    {
        // compute breaks and mesh size
        meshStart.resize(d);
        meshEnd.resize(d);
        curElement.resize(d);
        breaks.reserve(d);
        for (int i=0; i < d; ++i) 
        {
            breaks.push_back( m_basis.component(i).domain()->breaks() );
            // for n breaks, we have n-1 elements (spans)
            meshEnd[i]   = breaks[i].end() - 1;
            curElement[i] = meshStart[i] = breaks[i].begin();

            if (meshEnd[i] == meshStart[i])
                m_isGood = false;
        }
        
        // Set to one quadrature point by default
        m_quadrature.setNodes( gsVector<int>::Ones(d) );

        if (m_isGood)
            update();
    }

    // Documentation in gsDomainIterator.h
    bool next()
    {
        m_isGood = m_isGood && nextLexicographic(curElement, meshStart, meshEnd);
        if (m_isGood)
            update();
        return m_isGood;
    }

    // Documentation in gsDomainIterator.h
    void reset()
    {
        curElement = meshStart;
        m_isGood = ( meshEnd.array() != meshStart.array() ).all() ;
        if (m_isGood)
            update();
    }

    /// return the tensor index of the current element
    gsVector<unsigned, D> index() const
    {
        gsVector<unsigned, D> curr_index(d);
        for (int i = 0; i < d; ++i)
            curr_index[i]  = curElement[i] - breaks[i].begin();
        return curr_index; 
    }

    // Documentation in gsDomainIterator.h
    // compute a tensor Gauss quadrature rule of the given order for
    // the domain elements
    void computeQuadratureRule(const gsVector<int>& numIntNodes)
    {
        m_quadrature.setNodes(numIntNodes);
        m_quadrature.mapTo(lower, upper, this->quNodes, this->quWeights);
    }

    // get the basis function indices which are active in the current element
    void getActiveFunctions(gsMatrix<unsigned>& act)
    {
        m_basis.active_into(center, act);
    }

    const gsMatrix<unsigned>& computeActiveFunctions()
    {
        m_basis.active_into(center, this->activeFuncs);
        return this->activeFuncs;
    }

    void getVertices(gsMatrix<T>& result) 
    {
        result.resize( D, 1 << D); 

        gsVector<T,D> v, l, u;
        l.setZero();
        u.setOnes();
        v.setZero();
        int r = 0;
        do {
            for ( int i = 0; i< D; ++i)
                result(i,r) = ( v[i] ? upper[i] : lower[i] );
        }
        while ( nextCubeVertex(v, l, u) );
    }
    
    const gsVector<T> & lowerCorner() const
    { return lower; }

    const gsVector<T> & upperCorner() const
    { return upper; }

private:

    /// Computes lower, upper and center point of the current element, maps the reference
    /// quadrature nodes and weights to the current element, and computes the
    /// active functions.
    void update()
    {
        for (int i = 0; i < d; ++i)
        {
            lower[i]  = *curElement[i];
            upper[i]  = *(curElement[i]+1);
            center[i] = T(0.5) * (lower[i] + upper[i]);
        }

        // Update quadrature rule
        m_quadrature.mapTo(lower, upper, this->quNodes, this->quWeights);
        computeActiveFunctions();
    }

// Data members
public:
    using gsDomainIterator<T>::center;

protected:
    using gsDomainIterator<T>::m_basis;
    using gsDomainIterator<T>::m_isGood;

private:
    // the dimension of the parameter space
    int d;

    // coordinates of the grid cell boundaries
    std::vector< std::vector<T> > breaks;

    // Quadrature rule
    gsGaussRule<T> m_quadrature;

    // Extent of the tensor grid
    gsVector<uiter, D> meshStart, meshEnd;

    // Current element as pointers to it's supporting mesh-lines
    gsVector<uiter, D> curElement;

    // parameter coordinates of current grid cell
    gsVector<T> lower, upper;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
}; // class gsTensorDomainIterator


}; // namespace gismo
