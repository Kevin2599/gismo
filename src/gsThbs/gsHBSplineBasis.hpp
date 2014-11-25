
#pragma once 


namespace gismo
{

template<unsigned d, class T>
std::ostream & gsHBSplineBasis<d,T>::print(std::ostream &os) const
{
    os << "Hierarchical B-spline ";
    gsHTensorBasis<d,T>::printBasic(os);
    //this->printCharMatrix(os);
    return os;
}


template<unsigned d, class T>
void gsHBSplineBasis<d,T>::initialize()
{
    // Sets everything related to gsHTensorBasis
    // this->update_structure(); // base class update
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::evalSingle_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    result.resize(1,u.cols() );
    int lvl = this->levelOf(i);
    this->m_bases[lvl]->evalSingle_into( 
        this->m_xmatrix[lvl][ i - this->m_xmatrix_offset[lvl] ], u, result);
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::derivSingle_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    result.resize(1,u.cols() );
    int lvl = this->levelOf(i);
    this->m_bases[lvl]->derivSingle_into( 
        this->m_xmatrix[lvl][ i - this->m_xmatrix_offset[lvl] ], u, result);
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::deriv2Single_into(unsigned i, const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    int lvl = this->levelOf(i);
    this->m_bases[lvl]->deriv2Single_into( 
        this->m_xmatrix[lvl][ i - this->m_xmatrix_offset[lvl] ], u, result);
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::eval_into(const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    gsVector<unsigned> nact;
    gsMatrix<unsigned> act;
    // Get the number of active functions
    this->numActive(u, nact);
    // Set everything to zero
    result.setZero(nact.maxCoeff(), u.cols());
    
    gsMatrix<T> curr_res;
    for(index_t j = 0; j < u.cols(); j++) // for all points
    {
        const gsMatrix<T> & curr_u =  u.col(j);
        
        // Compute the indices of active functions on curr_u
        gsHBSplineBasis<d,T>::active_into(curr_u, act);
        
        // for all actives on the point
        for(index_t i = 0; i != act.rows(); i++) 
        {
            gsHBSplineBasis<d,T>::evalSingle_into( act(i,0), curr_u, curr_res );
            result(i,j) = curr_res(0,0);
        }
    }
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::deriv_into(const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    gsVector<unsigned> nact;
    gsMatrix<unsigned> act;
    // Get the number of active functions
    this->numActive(u, nact);
    // Set everything to zero
    result.setZero( d*nact.maxCoeff(), u.cols() );
    
    gsMatrix<T> curr_res;
    for(index_t j = 0; j < u.cols(); j++) // for all points
    {
        const gsMatrix<T> & curr_u =  u.col(j);
        
        // Compute the indices of active functions on curr_u
        gsHBSplineBasis<d,T>::active_into(curr_u, act);
        
        // for all actives on the point
            for(index_t i = 0; i < act.rows(); i++) 
            {
                gsHBSplineBasis<d,T>::derivSingle_into( act(i,0), curr_u, curr_res );
                result.block(i*d,j,d,1).noalias() = curr_res;
            }
    }
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::deriv2_into(const gsMatrix<T> & u, gsMatrix<T>& result) const
{
    gsVector<unsigned> nact;
    gsMatrix<unsigned> act;

    const int blocksize = d*(d + 1)/2;

    // Get the number of active functions
    this->numActive(u, nact);
    result.resize( blocksize * nact.maxCoeff(), u.cols() );
    // Set everything to zero
    result.setZero();

    // Current result for u.col(j)
    gsMatrix<T> curr_res;

    for(index_t j = 0; j < u.cols(); j++) // for all points
    {
        const gsMatrix<T> & curr_u =  u.col(j);
        
        // Compute the indices of active functions on curr_u
        gsHBSplineBasis<d,T>::active_into(curr_u, act);
        
        // for all actives at the point
        for(index_t i = 0; i < act.rows(); i++) 
        {
            gsHBSplineBasis<d,T>::deriv2Single_into( act(i,0), curr_u, curr_res );
            result.block(i*blocksize,j,blocksize,1) = curr_res;
        }
    }
}

template<unsigned d, class T>
void gsHBSplineBasis<d,T>::transferbyLvl (std::vector<gsMatrix<T> >& result){
    //std::vector< gsMatrix<T> > result;
    result.clear();
    gsVector<unsigned> level;
    gsMatrix<unsigned> b1, b2;//boxes in highes level numbering
    this->m_tree.getBoxesInLevelIndex(b1,b2,level);//return boxes in level indices
    gsTensorBSplineBasis<d,T, gsCompactKnotVector<T> > T_0_copy = this->tensorLevel(0);
    std::vector< gsSparseMatrix<T,RowMajor> > transfer;
    transfer.resize(this->get_max_inserted_level() );
    for(int i = 0; i < this->get_max_inserted_level();i++){
        //T_0_copy.uniformRefine_withTransfer(transfer[i], 1);
        std::vector<std::vector<T> > knots;
        for(unsigned int dim = 0l; dim < d;dim++)
        {
            gsCompactKnotVector<T> & ckv = this->m_bases[i]->component(dim).knots();
            gsCompactKnotVector<T> & fkv = this->m_bases[i + 1]->component(dim).knots();

            std::vector<T> dirKnots;
            this->_differenceBetweenKnotVectors(ckv, 0, ckv.uSize() - 1,
                                          fkv, 0, fkv.uSize() - 1,
                                          dirKnots);
            knots.push_back(dirKnots);
        }
        T_0_copy.refine_withTransfer(transfer[i], knots);
    }

    for(int j = 0; j < this->get_max_inserted_level();j++){
        std::vector<gsSortedVector<unsigned> >x_mat_old_0;
        this->setActiveToLvl(j,x_mat_old_0);
        std::vector<gsSortedVector<unsigned> > x_matrix_lvl;
        this->setActiveToLvl(j+1,x_matrix_lvl);

        gsMatrix<T> crs = this->coarsening(x_mat_old_0, x_matrix_lvl, transfer[j]);
        result.push_back(crs);
    }
    //return result;
}


template<unsigned d, class T>
gsMatrix<T> gsHBSplineBasis<d,T>::coarsening( std::vector<gsSortedVector<unsigned> > old,  std::vector<gsSortedVector<unsigned> > n, const gsSparseMatrix<T,RowMajor> & transfer){
    int size1= 0;int size2 = 0;
    int glob_numb = 0;//continous numbering of hierarchical basis
    for(unsigned int i =0; i< old.size();i++){//count the number of basis functions in old basis
        size1 += old[i].size();
    }
    for(unsigned int i =0; i< n.size();i++){//count the number of basis functions in new basis
        size2 += n[i].size();
    }
    gsMatrix<T> result(size2,size1);
    result.setZero();

    //gsMatrix<T> transferDense = transfer;
    gsSparseMatrix<T,ColMajor> temptransfer =  transfer;
//    std::cout<<"size1 "<<size1<<" size2 "<<size2<<std::endl;
//    std::cout<<"temptransfer.rows"<< temptransfer.rows()<<" cols "<<temptransfer.cols();
    for (unsigned int i = 0; i < old.size(); i++)//iteration through the levels of the old basis
    {
        // find starting index of level i in new basis
        int start_lv_i = 0;
        for(unsigned int l =0; l < i; l++)
        {
            start_lv_i += n[l].size();
        }

        for (unsigned int j = 0; j < old[i].size();j++)//iteration through the basis functions in the given level
        {
//            std::cout<<"function "<<j<<std::endl;
            const unsigned old_ij = old[i][j];  // tensor product index

            if( n[i].bContains(old_ij) )//it he basis function was not refined
            {
                result(start_lv_i + std::distance(n[i].begin(), n[i].find_it_or_fail(old_ij) ),glob_numb ) = 1;//settign the coefficient of the not refined basis function to 1
            }
            else
            {
//          TODO delete if the spafce matrix version is tested
//                for(int k = 0; k < transferDense.rows(); k++)//basis function was refined->looking for the coeficinets from the global transfer matrix
//                {
//                    if(transferDense(k, old_ij) != 0)//if the coefficient is non zero we find the coresponding function in n
//                    {
//                        const int pos = start_lv_i + n[i].size() + std::distance(n[i+1].begin(), n[i+1].find_it_or_fail(k));
//                        result(pos,glob_numb) = transferDense(k, old_ij);
//                    }
//                }
                for(typename gsSparseMatrix<T,ColMajor>::InnerIterator k(temptransfer,old_ij); k; ++k)//basis function was refined->looking for the coeficinets from the global transfer matrix
                {

                    //if(transferDense(k, old_ij) != 0)//if the coefficient is non zero we find the coresponding function in n
                    //{
                    const int pos = start_lv_i + n[i].size() + std::distance(n[i+1].begin(), n[i+1].find_it_or_fail(k.row()));
                    result(pos,glob_numb) = k.value();//transferDense(k, old_ij);
                    //}
                }
            }
            glob_numb++;
        }
    }
    return result;
}

template<unsigned d, class T>
gsMatrix<T> gsHBSplineBasis<d,T>::coarsening_direct( std::vector<gsSortedVector<unsigned> > old,std::vector<gsSortedVector<unsigned> > n, const std::vector<gsSparseMatrix<T,RowMajor> >& transfer){
    int size1= 0;int size2 = 0;
    int glob_numb = 0;//continous numbering of hierarchical basis
    for(unsigned int i =0; i< old.size();i++){//count the number of basis functions in old basis
        size1 += old[i].size();
    }
    for(unsigned int i =0; i< n.size();i++){//count the number of basis functions in new basis
        size2 += n[i].size();
    }
    gsMatrix<T> result(size2,size1);
    result.setZero();


//    std::vector<gsMatrix<T> > transferDense;// = transfer;
//    transferDense.resize(transfer.size());
//    for (unsigned int i = 0; i < transfer.size();i++){
//        transferDense[i] = transfer[i];
//    }
    std::vector<gsSparseMatrix<T,ColMajor> > temptransfer;// = transfer;
    temptransfer.resize(transfer.size());
    for (unsigned int i = 0; i < transfer.size();i++){
        temptransfer[i] = transfer[i];
    }

    for (unsigned int i = 0; i < old.size(); i++)//iteration through the levels of the old basis
    {
        // find starting index of level i in new basis
        int start_lv_i = 0;
        for(unsigned int l =0; l < i; l++)
        {
            start_lv_i += n[l].size();
        }


        for (unsigned int j = 0; j < old[i].size();j++)//iteration through the basis functions in the given level
        {

            int start_lv_i = 0;
            for(unsigned int l =0; l < i; l++)
            {
                start_lv_i += n[l].size();
            }
            const unsigned old_ij = old[i][j];  // tensor product index

            if( n[i].bContains(old_ij) )//it he basis function was not refined
            {
                result(start_lv_i + std::distance(n[i].begin(), n[i].find_it_or_fail(old_ij) ),glob_numb ) = 1;//settign the coefficient of the not refined basis function to 1
            }
            else
            {
                std::vector<lvl_coef> coeffs;
                lvl_coef temp;
                temp.pos = old_ij;
                temp.coef = 1;
                temp.lvl = i;
                coeffs.push_back(temp);
                while(!coeffs.empty()){
                    start_lv_i = 0;
                    for(unsigned int l =0; l < coeffs[0].lvl; l++)
                    {
                        start_lv_i += n[l].size();
                    }
//                    for(int k = 0; k < transferDense[coeffs[0].lvl].rows(); k++)//basis function was refined->looking for the coeficinets from the global transfer matrix
//                    {
//                        if(transferDense[coeffs[0].lvl](k, coeffs[0].pos) != 0)
//                        {
//                            if(n[coeffs[0].lvl+1].bContains(k))
//                            {
//                                const int pos = start_lv_i + n[coeffs[0].lvl].size() + std::distance(n[coeffs[0].lvl+1].begin(), n[coeffs[0].lvl+1].find_it_or_fail(k));
//                                //double ppp =  transferDense[coeffs[0].lvl](k, coeffs[0].pos);
//                                result(pos,glob_numb) += coeffs[0].coef * transferDense[coeffs[0].lvl](k, coeffs[0].pos);//transferDense[coeffs[0].lvl](k, coeffs[0].pos);
//                            }else{
//                                temp.pos = k;
//                                temp.coef = transferDense[coeffs[0].lvl](k, coeffs[0].pos) * coeffs[0].coef;
//                                temp.lvl = coeffs[0].lvl+1;
//                                coeffs.push_back(temp);
//                            }
//                        }
//                    }
                    for(typename gsSparseMatrix<T,ColMajor>::InnerIterator k(temptransfer[coeffs[0].lvl],coeffs[0].pos); k; ++k)//basis function was refined->looking for the coeficinets from the global transfer matrix
                    {
                        if(n[coeffs[0].lvl+1].bContains(k.row()))
                        {
                            const int pos = start_lv_i + n[coeffs[0].lvl].size() + std::distance(n[coeffs[0].lvl+1].begin(), n[coeffs[0].lvl+1].find_it_or_fail(k.row()));
                            //double ppp =  transferDense[coeffs[0].lvl](k, coeffs[0].pos);
                            result(pos,glob_numb) += coeffs[0].coef * temptransfer[coeffs[0].lvl](k.row(), coeffs[0].pos);//transferDense[coeffs[0].lvl](k, coeffs[0].pos);
                        }else
                        {
                            temp.pos = k.row();
                            temp.coef = temptransfer[coeffs[0].lvl](k.row(), coeffs[0].pos) * coeffs[0].coef;
                            temp.lvl = coeffs[0].lvl+1;
                            coeffs.push_back(temp);
                        }
                    }

//                    for(unsigned int ii = 0; ii < coeffs.size();ii++){
//                        std::cout<<"( "<< coeffs[ii].pos<<" , "<<coeffs[ii].coef<<" , "<<coeffs[ii].lvl<<" )"<<std::endl;
//                    }
                    coeffs.erase(coeffs.begin());
                }
            }
            glob_numb++;
        }
    }
    return result;
}
    
}// namespace gismo
