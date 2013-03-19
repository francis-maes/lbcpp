//===========================================================================
/*!
 *  \brief PCA
 *
 *  \author T.Glasmachers, Christian Igel
 *  \date 2010-2011
 *
 *  <BR><HR>
 *  This file is part of Shark. This library is free software;
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software
 *  Foundation; either version 3, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================
#include <shark/LinAlg/eigenvalues.h>
#include <shark/LinAlg/VectorStatistics.h>
#include <shark/Algorithms/Trainers/PCA.h>

using namespace shark;

	/// Set the input data, which is stored in the PCA object.
void PCA::setData(UnlabeledData<RealVector> const& inputs) {
	SHARK_CHECK(inputs.numberOfElements() >= 2, "[PCA::train] input needs to contain at least two points");
	m_l = inputs.numberOfElements(); ///< number of data points
	PCAAlgorithm algorithm = m_algorithm;
	m_n = dataDimension(inputs); 
	
	if(algorithm == AUTO)  {
		if(m_n > m_l) algorithm = SMALL_SAMPLE; // more attributes than data points
		else algorithm = STANDARD;
	}
	
	// decompose covariance matrix
	if(algorithm == STANDARD) { // standard case
		RealMatrix S(m_n,m_n);//covariance matrix
		meanvar(inputs,m_mean,S);
		//~ symmRankKUpdate(trans(X0),S);
		//~ S /= m_l;
		m_eigenvalues.resize(m_n);
		m_eigenvectors.resize(m_n, m_n);
		eigensymm(S, m_eigenvectors, m_eigenvalues);
	} else {
		//let X0 be the design matrix having all inputs as rows
		//we want to avoid to form it directly but us it's batch represntation in the dataset
		m_mean = shark::mean(inputs);
		RealMatrix S(m_l,m_l,0.0);//S=X0 X0^T
		//as every batch is only  afraction of all samples, we have to loop through all
		//combinations of the batches of first and second argument and calculate a block of S
		//so if X0^T = (B0^T,B1^T)
		//than S = B0 B0^T B0 B1^T
		//               B1 B0^T  B1 B1^T
		std::size_t start1 = 0;
		for(std::size_t b1 = 0; b1 != inputs.numberOfBatches(); ++b1){
			std::size_t batchSize1 = inputs.batch(b1).size1();
			RealMatrix X1 = inputs.batch(b1)-repeat(m_mean,batchSize1);
			std::size_t start2 = 0;
			//calculate off-diagonal blocks
			//and the block X2 X1^T is the transpose of X1X2^T and thus can bee calculated for free.
			for(std::size_t b2 = 0; b2 != b1; ++b2){
				std::size_t batchSize2 = inputs.batch(b2).size1();
				RealMatrix X2 = inputs.batch(b2)-repeat(m_mean,batchSize2);
				RealSubMatrix X1X2T= subrange(S,start1,start1+batchSize1,start2,start2+batchSize2);
				RealSubMatrix X2X1T= subrange(S,start2,start2+batchSize2,start1,start1+batchSize1);
				fast_prod(X1,trans(X2),X1X2T);// X1 X2^T
				noalias(X2X1T) = trans(X1X2T);// X2 X1^T
				start2+=batchSize2;
			}
			//diagonal block
			RealSubMatrix X1X1T= subrange(S,start1,start1+batchSize1,start1,start1+batchSize1);
			symmRankKUpdate(X1,X1X1T);
			start1+=batchSize1;
		}
		S /= m_l;
		//~ std::cout<<S<<std::endl;
		m_eigenvalues.resize(m_l);
		m_eigenvectors.resize(m_n, m_l);
		zero(m_eigenvectors);
		RealMatrix U(m_l, m_l);
		eigensymm(S, U, m_eigenvalues);
		//~ std::cout<<"pca U:"<<U<<std::endl;
		// compute true eigenvectors
		//eigenv=X0^T U
		std::size_t batchStart  = 0;
		for(std::size_t b = 0; b != inputs.numberOfBatches(); ++b){
			std::size_t batchSize = inputs.batch(b).size1();
			std::size_t batchEnd = batchStart+batchSize;
			RealMatrix X = inputs.batch(b)-repeat(m_mean,batchSize);
			fast_prod(trans(X),rows(U,batchStart,batchEnd),m_eigenvectors,true);
			batchStart = batchEnd;
		}
		//fast_prod(trans(X0),U,m_eigenvectors);
		//~ std::cout<<"Eig:"<<m_eigenvectors<<std::endl;
		//normalize
		for(std::size_t i=0; i != m_l; i++)
			column(m_eigenvectors, i) /= norm_2(column(m_eigenvectors, i));
	}
}

//! Returns a model mapping the original data to the
//! m-dimensional PCA coordinate system.
void PCA::encoder(LinearModel<>& model, std::size_t m) {
	if(!m) m = std::min(m_n,m_l);
	
	RealMatrix A = trans(columns(m_eigenvectors, 0, m) );
	RealVector offset(A.size1()); 
	fast_prod(A, m_mean, offset, false, -1.0);
	if(m_whitening){
		for(std::size_t i=0; i<A.size1(); i++) {
			row(A, i) = row(A, i) / std::sqrt(m_eigenvalues(i));
			offset(i) /= std::sqrt(m_eigenvalues(i));
		}
	}
	model.setStructure(A, offset);
}

//! Returns a model mapping encoded data from the
//! m-dimensional PCA coordinate system back to the
//! n-dimensional original coordinate system.
void PCA::decoder(LinearModel<>& model, std::size_t m) {
	if(!m) m = std::min(m_n,m_l);
	if( m == m_n && !m_whitening){
		model.setStructure(m_eigenvectors, m_mean);
	}
	RealMatrix A = columns(m_eigenvectors, 0, m);
	if(m_whitening){
		for(std::size_t i=0; i<A.size2(); i++) {
			column(A, i) = column(A, i) * std::sqrt(m_eigenvalues(i));
		}
	}

	model.setStructure(A, m_mean);
}
