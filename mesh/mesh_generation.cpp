#include "mesh_generation.h"

#include "coordinate.h"
#include "dataset.h"
#include "loader.h"
#include "segmentation/cubeloader.h"
#include "segmentation/segmentation.h"
#include "skeleton/skeletonizer.h"

#include "vtkMarchingCubesTriangleCases.h"

#include <QApplication>
#include <QFuture>
#include <QFutureWatcher>
#include <QProgressDialog>
#include <QObject>
#include <QtConcurrentMap>

#include <snappy.h>

#include <unordered_map>

template<typename T, typename U>
void marchingCubes(std::unordered_map<U, std::unordered_map<floatCoordinate, int>> & obj2points, std::unordered_map<U, std::vector<unsigned int>> & obj2faces, std::unordered_map<std::uint64_t, std::size_t> & obj2idCounter, const std::vector<T> & data, const std::unordered_map<T, U> & soid2oid, const std::array<double, 3> & origin, const std::array<double, 3> & dims, const std::array<double, 3> & spacing, const std::array<double, 6> & extent) {
    const auto triCases = vtkMarchingCubesTriangleCases::GetCases();

    static const std::array<int, 8> CASE_MASK{1,2,4,8,16,32,64,128};
    static const std::array<std::array<int, 2>, 12> edges{{{0,1}, {1,2}, {3,2}, {0,3}, {4,5}, {5,6}, {7,6}, {4,7}, {0,4}, {1,5}, {3,7}, {2,6}}};

    const auto rowSize = dims[0];
    const auto sliceSize = rowSize * dims[1];

    for (std::size_t z = 0; z < (dims[2] - 1); ++z) {
        const auto zOffset = z * sliceSize;
        std::array<std::array<double, 3>, 8> pts;
        pts[0][2] = origin[2] + (z + extent[4]) * spacing[2];
        const auto znext = pts[0][2] + spacing[2];
        for (std::size_t y = 0; y < (dims[1] - 1); ++y) {
            const auto yOffset = y * rowSize;
            pts[0][1] = origin[1] + (y + extent[2]) * spacing[1];
            const auto ynext = pts[0][1] + spacing[1];
            for (std::size_t x = 0; x < (dims[0] - 1); ++x) {
                // get scalar values
                const auto idx = x + yOffset + zOffset;
                std::array<T, 8> cubeVals;
                cubeVals[0] = data[idx];
                cubeVals[1] = data[idx+1];
                cubeVals[2] = data[idx+1 + dims[0]];
                cubeVals[3] = data[idx + dims[0]];
                cubeVals[4] = data[idx + sliceSize];
                cubeVals[5] = data[idx+1 + sliceSize];
                cubeVals[6] = data[idx+1 + dims[0] + sliceSize];
                cubeVals[7] = data[idx + dims[0] + sliceSize];

                // create voxel points
                pts[0][0] = origin[0] + (x + extent[0]) * spacing[0];
                const auto xnext = pts[0][0] + spacing[0];

                pts[1][0] = xnext;
                pts[1][1] = pts[0][1];
                pts[1][2] = pts[0][2];

                pts[2][0] = xnext;
                pts[2][1] = ynext;
                pts[2][2] = pts[0][2];

                pts[3][0] = pts[0][0];
                pts[3][1] = ynext;
                pts[3][2] = pts[0][2];

                pts[4][0] = pts[0][0];
                pts[4][1] = pts[0][1];
                pts[4][2] = znext;

                pts[5][0] = xnext;
                pts[5][1] = pts[0][1];
                pts[5][2] = znext;

                pts[6][0] = xnext;
                pts[6][1] = ynext;
                pts[6][2] = znext;

                pts[7][0] = pts[0][0];
                pts[7][1] = ynext;
                pts[7][2] = znext;

                std::unordered_map<U, std::uint64_t> obj2index;
                // Build the case table
                for (std::size_t pi = 0; pi < 8; ++pi) {
                    // for discrete marching cubes, we are looking for an
                    // exact match of a scalar at a vertex to a value
                    if (auto it = soid2oid.find(cubeVals[pi]); it != std::end(soid2oid)) {
                        for (auto & oindex : Segmentation::singleton().subobjectFromId(it->first, {}).objects) {
                            obj2index[Segmentation::singleton().objects[oindex].id] |= CASE_MASK[pi];
                        }
                    }
                }
                for (auto pair : obj2index) {
                    const auto oid = pair.first;
                    auto & points = obj2points[oid];
                    auto & faces = obj2faces[oid];
                    auto & idCounter = obj2idCounter[oid];
                    const auto index = pair.second;
                    if (index != 0 && index != 255) {// no surface
                        const auto & triCase = triCases[index];
                        for (auto edge = triCase.edges; edge[0] > -1; edge += 3) {
                            std::array<std::size_t, 3> ptIds;
                            for (std::size_t vi = 0; vi < 3; vi++) {// insert triangle
                                const auto vert = edges[edge[vi]];
                                // for discrete marching cubes, the interpolation point is always 0.5.
                                const auto t = 0.5;
                                const auto x1 = pts[vert[0]];
                                const auto x2 = pts[vert[1]];
                                std::array<float, 3> vertex;
                                vertex[0] = x1[0] + t * (x2[0] - x1[0]);
                                vertex[1] = x1[1] + t * (x2[1] - x1[1]);
                                vertex[2] = x1[2] + t * (x2[2] - x1[2]);
                                floatCoordinate coord{vertex[0], vertex[1], vertex[2]};
                                if (points.find(coord) == std::end(points)) {
                                    points[coord] = ptIds[vi] = idCounter++;// add point
                                } else {
                                    ptIds[vi] = points[coord];
                                }
                            }
                            if (ptIds[0] != ptIds[1] && ptIds[0] != ptIds[2] && ptIds[1] != ptIds[2] ) {// check for degenerate triangle
                                faces.emplace_back(ptIds[0]);
                                faces.emplace_back(ptIds[1]);
                                faces.emplace_back(ptIds[2]);
                            }
                        }
                    }
                }
            }
        }
    }
}

auto generateMeshForSubobjectID(const std::unordered_map<std::uint64_t, std::uint64_t> & soid2oid, const std::vector<std::uint64_t> & objects, const Loader::Worker::SnappyCache & cubes, QProgressDialog & progress) {
    std::vector<std::unordered_map<std::uint64_t, std::unordered_map<floatCoordinate, int>>> obj2totalpoints(cubes.size());
    std::vector<std::unordered_map<std::uint64_t, std::vector<unsigned int>>> obj2totalfaces(cubes.size());
    const auto processCube = [&](const auto & val){
        const auto id = val.first;
        const auto & pair = *val.second;
        auto & obj2points = obj2totalpoints[id];
        auto & obj2faces = obj2totalfaces[id];
        std::unordered_map<std::uint64_t, std::size_t> obj2idCounter;
        const std::size_t cubeEdgeLen = Dataset::current().cubeEdgeLength;
        const std::size_t size = std::pow(cubeEdgeLen, 3);

        std::unordered_map<CoordOfCube, std::vector<std::uint64_t>> extractedCubes;// local lookup
        auto extractedCubeForCoord = [&cubes, &extractedCubes, size](const auto & coord) -> auto & {
            auto & cube = extractedCubes[coord];
            if (cube.empty()) {
                cube.resize(size);
                auto findIt = cubes.find(coord);
                if (findIt != std::end(cubes)) {
                    if (!snappy::RawUncompress(findIt->second.c_str(), findIt->second.size(), reinterpret_cast<char *>(cube.data()))) {
                        std::runtime_error("failed to extract snappy cube in generateMeshForSubobjectID");
                    }
                } else {
                    std::fill(std::begin(cube), std::end(cube), 0);// dummy data for missing or broken cubes in snappy cache
                }
            }
            return cube;
        };

        const auto scale = Dataset::current().scale * Dataset::current().magnification;
        const auto cubeCoord = scale.componentMul(pair.first.cube2Global(cubeEdgeLen, 1));

        const std::array<double, 3> dims{{cubeEdgeLen * 1.0, cubeEdgeLen * 1.0, cubeEdgeLen * 1.0}};
        const std::array<double, 3> origin{{cubeCoord.x, cubeCoord.y, cubeCoord.z}};
        const std::array<double, 3> spacing{{scale.x, scale.y, scale.z}};
        const std::array<double, 6> extent{{0, dims[0], 0, dims[1], 0, dims[2]}};

        marchingCubes(obj2points, obj2faces, obj2idCounter, extractedCubeForCoord(pair.first), soid2oid, origin, dims, spacing, extent);
        for (std::size_t i = 0; i < 6; ++i) {
            const std::array<double, 3> dims{{i < 2 ? 2.0 : cubeEdgeLen + 2, i % 4 < 2 ? cubeEdgeLen + 2 : 2.0, i < 4 ? cubeEdgeLen + 2 : 2.0}};
            const floatCoordinate unscaledOrigin(pair.first.cube2Global(cubeEdgeLen, 1) + floatCoordinate(i == 1 ? 127 : -1, i == 3 ? 127 : -1, i == 5 ? 127 : -1));
            const std::array<double, 6> extent{{0, dims[0], 0, dims[1], 0, dims[2]}};

            std::vector<std::uint64_t> data(2 * std::pow(cubeEdgeLen + 2, 2));

            const auto rowSize = dims[0];
            const auto sliceSize = rowSize * dims[1];
            for (std::size_t z = 0; z < dims[2]; ++z) {
                for (std::size_t y = 0; y < dims[1]; ++y) {
                    for (std::size_t x = 0; x < dims[0]; ++x) {
                        const Coordinate globalPos = unscaledOrigin + Coordinate(x, y, z);
                        if (globalPos.x < 0 || globalPos.y < 0 || globalPos.z < 0) {
                            continue;
                        }
                        const CoordOfCube coord = globalPos.cube(cubeEdgeLen, 1);
                        const auto inCube = globalPos.insideCube(cubeEdgeLen, 1);
                        auto & cube = extractedCubeForCoord(coord);
                        data[z * sliceSize + y * rowSize + x] = boost::multi_array_ref<uint64_t, 3>(reinterpret_cast<std::uint64_t *>(cube.data()), boost::extents[cubeEdgeLen][cubeEdgeLen][cubeEdgeLen])[inCube.z][inCube.y][inCube.x];
                    }
                }
            }
            const auto scaledOrigin = scale.componentMul(unscaledOrigin);
            marchingCubes(obj2points, obj2faces, obj2idCounter, data, soid2oid, {{scaledOrigin.x, scaledOrigin.y, scaledOrigin.z}}, dims, spacing, extent);
        }
    };
    std::string s;
    std::vector<std::pair<std::size_t, decltype(std::cbegin(cubes))>> threadids;
    std::size_t i = 0;
    for (auto it = std::begin(cubes); it != std::end(cubes); ++it) {
        threadids.emplace_back(i, it);
        ++i;
    }

    QEventLoop pause;
    QFutureWatcher<void> watcher;
    QObject::connect(&watcher, &decltype(watcher)::progressRangeChanged, &progress, &QProgressDialog::setRange);
    QObject::connect(&watcher, &decltype(watcher)::progressValueChanged, &progress, &QProgressDialog::setValue);
    QObject::connect(&watcher, &decltype(watcher)::finished, [&pause](){ pause.exit();} );
    QObject::connect(&progress, &QProgressDialog::canceled, &watcher, &decltype(watcher)::cancel);
    watcher.setFuture(QtConcurrent::map(threadids, processCube));
    pause.exec();

    progress.setLabelText(progress.labelText() + QObject::tr("\nFinalizing …"));
    progress.setRange(0, objects.size());
    int value{0};

    for (const auto oid : objects) {
        if (progress.wasCanceled()) {
            break;
        }

        QVector<float> verts;
        std::vector<std::size_t> offsets;
        for (const auto & elempoints : obj2totalpoints) {
            if (auto pit = elempoints.find(oid); pit != std::end(elempoints)) {
                const auto offset = verts.size();
                offsets.emplace_back(offset / 3);
                verts.resize(offset + 3 * pit->second.size());
                for (const auto & pair : pit->second) {
                    verts[offset + 3 * pair.second    ] = pair.first.x;
                    verts[offset + 3 * pair.second + 1] = pair.first.y;
                    verts[offset + 3 * pair.second + 2] = pair.first.z;
                }
            }
        }
        QVector<unsigned int> faces;
        std::size_t offseti{0};
        for (const auto & elemfaces : obj2totalfaces) {
            if (auto pit = elemfaces.find(oid); pit != std::end(elemfaces)) {
                for (const auto & elem : pit->second) {
                    faces.push_back(offsets[offseti] + elem);
                }
                ++offseti;
            }
        }

        qDebug() << oid << ':' << (verts.size() / 3) << "→" << faces.size() / 3;

        QVector<float> normals;
        QVector<std::uint8_t> colors;
        QSignalBlocker blocker(Skeletonizer::singleton());
        Skeletonizer::singleton().addMeshToTree(oid, verts, normals, faces, colors, GL_TRIANGLES);
        progress.setValue(++value);
    }
    Skeletonizer::singleton().resetData();
}

void generateMeshesForSubobjectsOfSelectedObjects() {
    const auto & cubes = Loader::Controller::singleton().getAllModifiedCubes();
    const auto msg = QObject::tr("Generating meshes for %1 objects over %2 cubes").arg(Segmentation::singleton().selectedObjectsCount()).arg(cubes[0].size());
    QProgressDialog progress(msg, "Cancel", 0, Segmentation::singleton().selectedObjectsCount() * cubes[0].size(), QApplication::activeWindow());
    progress.setWindowModality(Qt::WindowModal);
    qDebug() << msg.toUtf8().constData();

    std::unordered_map<std::uint64_t, std::uint64_t> soids;
    std::vector<std::uint64_t> oids;
    for (const auto objectIndex : Segmentation::singleton().selectedObjectIndices) {
        const auto oid = Segmentation::singleton().objects[objectIndex].id;
        for (const auto & elem : Segmentation::singleton().objects[objectIndex].subobjects) {
            soids.emplace(elem.get().id, oid);
        }
        oids.emplace_back(oid);
    }
    generateMeshForSubobjectID(soids, oids, cubes[Dataset::current().magIndex], progress);
}
