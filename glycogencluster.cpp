#include "mesh.h"
#include "glycogen.h"
#include "glycogencluster.h"


namespace Clustering
{
	int GlycogenCluster::ID_COUNTER = 0;
	//--------------------------------------------------------------------------------
	//
	GlycogenCluster::GlycogenCluster()
	{
		GlycogenCluster::ID_COUNTER++;
		m_id = GlycogenCluster::ID_COUNTER;
		m_totalVolume = 0;
		m_sumNode.setX(0);
		m_sumNode.setY(0);
		m_sumNode.setZ(0);
	}

	//--------------------------------------------------------------------------------
	//
	GlycogenCluster::~GlycogenCluster()
	{

	}

	//--------------------------------------------------------------------------------
	//
	int GlycogenCluster::getID()
	{
		return m_id;
	}


	//--------------------------------------------------------------------------------
	//
	void GlycogenCluster::addNode(Glycogen* glycogen)
	{
		if (!glycogen)
			return;

		m_totalVolume += glycogen->getVolume();
		glycogen->setClusterID(m_id);

		m_sumNode.setX(m_sumNode.x() + glycogen->x());
		m_sumNode.setY(m_sumNode.y() + glycogen->y());
		m_sumNode.setZ(m_sumNode.z() + glycogen->z());

		m_glycogenMap[glycogen->getID()] = glycogen;
	}

	//--------------------------------------------------------------------------------
	//
	QVector3D GlycogenCluster::getAvgNode()
	{
		float n = m_glycogenMap.size();
		QVector3D avgNode(m_sumNode.x() / n, m_sumNode.y() / n, m_sumNode.z() / n);
		return avgNode;
	}

	//--------------------------------------------------------------------------------
	//
	bool GlycogenCluster::contains(int id)
	{
		auto found = m_glycogenMap.find(id);
		return found != m_glycogenMap.end();
	}
}