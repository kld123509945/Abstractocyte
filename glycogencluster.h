#ifndef GLYCOGENCLUSTER_H_
#define GLYCOGENCLUSTER_H_

#include <map>
#include <QVector3D>


namespace Clustering
{

	class GlycogenCluster
	{
	public:
		GlycogenCluster();
		~GlycogenCluster();

		int  getID();
		void addNode(Glycogen* glycogen);
		QVector3D getAvgNode();
		bool contains(int id);

	protected:
		int					   m_id;
		float				   m_totalVolume;
		QVector3D			   m_sumNode;
		std::map<int,Glycogen*> m_glycogenMap;
		static int			   ID_COUNTER;
	};

	
}

#endif // GLYCOGENCLUSTER_H_