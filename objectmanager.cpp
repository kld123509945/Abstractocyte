#include <chrono>
#include "objectmanager.h"

/*
 * m_objects -> object class for each object (astrocyte, dendrite, ..)
 *           -> get from this the indices of the mesh
 *           -> get the skeleton vertices
 *
 * m_buffer_data for ssbo initialization which I can take by looping over all m_objects
 */
ObjectManager::ObjectManager()
{
    m_indices_size = 0;
    m_skeleton_points_size = 0;
    m_limit = 20;
    m_vertex_offset = 0;
    m_mesh = new Mesh();

   importXML("://scripts/m3_astrocyte.xml");   // astrocyte  time:  79150.9 ms
   importXML("://scripts/m3_neurites.xml");    // neurites time:  28802 ms
}

ObjectManager::~ObjectManager()
{
    qDebug() << "~Mesh()";
    for (std::size_t i = 0; i != m_objects.size(); i++) {
        delete m_objects[i];
    }
}

bool ObjectManager::importXML(QString path)
{
    qDebug() << "Func: importXML";
    auto t1 = std::chrono::high_resolution_clock::now();

    QFile  *file = new QFile(path);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Could not open the file for reading";
        return false;
    }

    m_vertex_offset += m_mesh->getVerticesSize();

    QXmlStreamReader xml(file);
    while( !xml.atEnd() && !xml.hasError() ) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartDocument) {
            continue;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "o") {
                if (m_objects.size() > m_limit) {
                    qDebug() << "* Reached size limit.";
                    break;
                }
                Object *obj = NULL;

                parseObject(xml, obj); // fills the object with obj info
            } else if (xml.name() == "conn") {
                parseConnGraph(xml);
           }
        }
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> ms = t2 - t1;
    qDebug() << "time: " << ms.count() << "ms";
}

void ObjectManager::parseConnGraph(QXmlStreamReader &xml)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "conn") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    qDebug()  << "Parsing: " << xml.name();
    xml.readNext();
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "conn")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "l") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int nodeID1 = stringlist.at(0).toInt();
                int nodeID2 = stringlist.at(1).toInt();

                QVector2D edge_info = QVector2D(nodeID1, nodeID2);
                neurites_neurite_edge.push_back(edge_info);
            }
        } // if start element
        xml.readNext();
    } // while
}

// load the object with all its related informations
void ObjectManager::parseObject(QXmlStreamReader &xml, Object *obj)
{
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "o") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }
    qDebug() << xml.name();

    QString nameline  = xml.attributes().value("name").toString();
    QList<QString> nameList = nameline.split('_');
    QString name;
    for (int i = 0; i < nameList.size() - 1; ++i)
        name += nameList[i];

    int hvgxID = nameList[nameList.size() - 1].toInt();
    obj = new Object(name.toUtf8().constData(), hvgxID);

    QVector4D color = obj->getColor();
    // set ssbo with this ID to this color

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "o")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "mesh") {
                parseMesh(xml, obj);
            } else if (xml.name() == "skeleton") {
                parseSkeleton(xml, obj);
            } else if (xml.name() == "volume") {
                xml.readNext();
                int volume =  xml.text().toInt();
                obj->setVolume(volume);
                qDebug() << "volume: " << volume;
            } else if (xml.name() == "center") {
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble();
                float y = stringlist.at(1).toDouble();
                float z = stringlist.at(2).toDouble();
                qDebug() << "center: " << x << " " << y << " " << z;
                obj->setCenter(QVector4D(x/5.0, y/5.0, z/5.0, 0));
                // set ssbo with this center
            }
        } // if start element
        xml.readNext();
    } // while

    if (obj != NULL) {
         m_objects[hvgxID] =  obj;
    }

}


void ObjectManager::parseMesh(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "mesh") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }

    qDebug() << xml.name();
    xml.readNext();

    int vertices = 0;
    int faces = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "mesh")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++vertices;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x1 = stringlist.at(0).toDouble()/5.0;
                float y1 = stringlist.at(1).toDouble()/5.0;
                float z1 = stringlist.at(2).toDouble()/5.0;

                QVector4D mesh_vertex(x1, y1, z1,  obj->getHVGXID());
                struct VertexData v;
                v.mesh_vertex = mesh_vertex;

                // todo: get the skeleton vertex from the skeleton itself using an index
                // get the point branch knots as well
                if (stringlist.size() < 5) {
                    // place holder
                    v.skeleton_vertex = mesh_vertex;
                } else {
                    float x2 = stringlist.at(3).toDouble()/5.0;
                    float y2 = stringlist.at(4).toDouble()/5.0;
                    float z2 = stringlist.at(5).toDouble()/5.0;
                    QVector4D skeleton_vertex(x2, y2, z2, 0);
                    v.skeleton_vertex = skeleton_vertex;
                }
                m_mesh->addVertex(v);
            } else if (xml.name() == "f") {
                ++faces;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                int f1_index = stringlist.at(0).toInt()  + m_vertex_offset;
                int f2_index = stringlist.at(1).toInt()  + m_vertex_offset;
                int f3_index = stringlist.at(2).toInt()  + m_vertex_offset;

                if (! m_mesh->isValidFaces(f1_index, f2_index, f3_index) ) {
                    // error, break
                    qDebug() << "Error in obj file! " << obj->getName().data() << " " << obj->getHVGXID()
                             << " " << f1_index << " " << f2_index << " " << f3_index ;
                    delete obj;
                    break;
                }

                // add faces indices to object itself
                /* vertex 1 */
                obj->addTriangleIndex(f1_index - 1);
                /* vertex 2 */
                obj->addTriangleIndex(f2_index - 1);
                /* vertex 3 */
                obj->addTriangleIndex(f3_index - 1);
                m_indices_size += 3;
            } else if (xml.name() == "markers") {
                qDebug() << xml.name();
                xml.readNext();
                QStringList markersList = xml.text().toString().split(" ");
                m_mesh->MarkBleedingVertices(markersList, m_vertex_offset);
            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "vertices count: " << vertices << " faces: " << faces;
}

//
void ObjectManager::parseSkeleton(QXmlStreamReader &xml, Object *obj)
{
    // read skeleton vertices and their edges
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "skeleton") {
        qDebug() << "Called XML parseObejct without attribs";
        return;
    }
    qDebug() << xml.name();

    xml.readNext();

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "skeleton")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "nodes") { // make it v
                qDebug() << xml.name();
                parseSkeletonNodes(xml, obj);
            } else if (xml.name() == "points") {
                qDebug() << xml.name();
                parseSkeletonPoints(xml, obj);
            } else if (xml.name() == "branches") {
              // process branches
                parseBranch(xml, obj);
            }
        } // if start element
        xml.readNext();
    } // while
}

void ObjectManager::parseSkeletonNodes(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "nodes") {
        qDebug() << "Called XML parseSkeletonNodes without attribs";
        return;
    }
    qDebug() << xml.name();

    xml.readNext();

    int nodes = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "nodes")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++nodes;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble()/5.0;
                float y = stringlist.at(1).toDouble()/5.0;
                float z = stringlist.at(2).toDouble()/5.0;
                QVector3D node(x, y, z);
                obj->addSkeletonNode(node);
            }
        } // if start element
        xml.readNext();
    } // while


    qDebug() << "nodes count: " << nodes;
}

void ObjectManager::parseSkeletonPoints(QXmlStreamReader &xml, Object *obj)
{
    // read vertices and their faces into the mesh
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "points") {
        qDebug() << "Called XML parseSkeletonNodes without attribs";
        return;
    }
    qDebug() << xml.name();


    xml.readNext();

    int nodes = 0;

    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "points")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "v") {
                ++nodes;
                m_skeleton_points_size++;
                xml.readNext();
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 3) {
                    continue;
                }

                float x = stringlist.at(0).toDouble()/5.0;
                float y = stringlist.at(1).toDouble()/5.0;
                float z = stringlist.at(2).toDouble()/5.0;
                QVector3D node(x, y, z);
                obj->addSkeletonPoint(node);
            }
        } // if start element
        xml.readNext();
    } // while

    qDebug() << "nodes count: " << nodes;
}

void ObjectManager::parseBranch(QXmlStreamReader &xml, Object *obj)
{
    // b -> one branch
    // knots
    // points
    if (xml.tokenType() != QXmlStreamReader::StartElement && xml.name() != "branches") {
        qDebug() << "Called XML parseBranch without attribs";
        return;
    }

    qDebug() << xml.name();
    xml.readNext();

    SkeletonBranch *branch = NULL;
    // this object structure is not done
    while(!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "branches")) {
        // go to the next child of object node
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "b") {
                branch = new SkeletonBranch();
                int ID = xml.attributes().value("name").toInt();
            } else if (xml.name() == "knots") {
                if (branch == NULL) {
                    qDebug() << "branch null";
                    continue;
                }
                xml.readNext();
                // two integers for the skeleton end nodes IDs
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                if (stringlist.size() < 2) {
                    continue;
                }

                int knot1 = stringlist.at(0).toInt();
                int knot2 = stringlist.at(1).toInt();
                branch->addKnots(knot1, knot2);
            } else if (xml.name() == "points_ids") {
                if (branch == NULL || obj == NULL) {
                    qDebug() << "branch null or obj is NULL";
                    continue;
                }
                xml.readNext();
                // list of integers for points IDs
                QString coords = xml.text().toString();
                QStringList stringlist = coords.split(" ");
                branch->addPointsIndxs(stringlist);
                obj->addSkeletonBranch(branch);
            }
        } // if start element
        xml.readNext();
    } // while
}

int ObjectManager::getSkeletonPointsSize()
{
    return m_skeleton_points_size;
}

Mesh* ObjectManager::getMeshPointer()
{
    return m_mesh;
}

int ObjectManager::getMeshIndicesSize()
{
    return m_indices_size;
}

std::map<int, Object*>  ObjectManager::getObjectsMap()
{
    return m_objects;
}

std::vector<QVector2D> ObjectManager::getNeuritesEdges()
{
    return neurites_neurite_edge;
}

void ObjectManager::update_ssbo_data_layout1(QVector2D layout1, int hvgxID)
{
//    if (m_ssbo_data.size() < hvgxID)
//        return;

//    m_ssbo_data[hvgxID].layout1 = layout1;
}

void ObjectManager::update_ssbo_data_layout2(QVector2D layout2, int hvgxID)
{
//    if (m_ssbo_data.size() < hvgxID)
//        return;

//    m_ssbo_data[hvgxID].layout2 = layout2;
}

void ObjectManager::update_skeleton_layout1(QVector2D layout1, int node_index, int hvgxID)
{
//    // get the object -> get its skeleton -> update the layout
//    if (m_objects.find(hvgxID) == m_objects.end()) {
//        return;
//    }

//    Skeleton *skel = m_objects[hvgxID]->getSkeleton();
//    int nodes_offset = skel->getIndexOffset();
//    m_abstract_skel_nodes[node_index + nodes_offset].layout1 = layout1;
}

void ObjectManager::update_skeleton_layout2(QVector2D layout2, int node_index, int hvgxID)
{
//    // get the object -> get its skeleton -> update the layout
//    // get the object -> get its skeleton -> update the layout
//    if (m_objects.find(hvgxID) == m_objects.end()) {
//        qDebug() << "Object not found";
//        return;
//    }

//    Skeleton *skel = m_objects[hvgxID]->getSkeleton();
//    int nodes_offset = skel->getIndexOffset();
//    m_abstract_skel_nodes[node_index + nodes_offset].layout2 = layout2;
}

void ObjectManager::update_skeleton_layout3(QVector2D layout3,int node_index, int hvgxID)
{
//    // get the object -> get its skeleton -> update the layout
//    // get the object -> get its skeleton -> update the layout
//    if (m_objects.find(hvgxID) == m_objects.end()) {
//        return;
//    }

//    Skeleton *skel = m_objects[hvgxID]->getSkeleton();
//    int nodes_offset = skel->getIndexOffset();
//    m_abstract_skel_nodes[node_index + nodes_offset].layout3 = layout3;
}
