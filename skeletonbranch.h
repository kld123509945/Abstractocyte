#ifndef SKELETONBRANCH_H
#define SKELETONBRANCH_H

#include "mainopengl.h"

class SkeletonBranch
{
public:
    SkeletonBranch();

protected:
    std::vector<QVector3D>  m_vertices;
    std::vector<QVector2D>  m_lines;
};

#endif // SKELETONBRANCH_H
