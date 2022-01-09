#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "AssetsFetcher.h"
#include "utils/tinyobj_loader_c.h"
#include <math.h>

#ifdef __ANDROID__
#include <android_native_app_glue.h>
#endif

char *tex_files_short[] = { "home8" };
char *meshes_files_short[] = { "textPanel.obj" };

void AssetsFetcher__init(struct AssetsFetcher* self) {
    self->tex_files_short = tex_files_short;
    self->meshes_files_short = meshes_files_short;
    self->outAttribAllocated = false;
}

void AssetsFetcher__reset(struct AssetsFetcher* self) {
    if (self->outAttribAllocated) {
        tinyobj_attrib_free(self->outAttrib);
    }
}

/*
 important : this is a sort of conceptual inversion
 num_face = num faces * 3
 num_face_num_verts = num faces
 */
void AssetsFetcher__loadObj(struct AssetsFetcher* self, const char* filename, tinyobj_attrib_t *outAttrib) {
    AssetsFetcher__reset(self);
    float bmin[3];
    float bmax[3];
    AAsset* asset = AAssetManager_open(
            self->assetManager,
            filename,
            AASSET_MODE_BUFFER
    );
    off64_t length1 = AAsset_getLength(asset);
    void* rawData = (void*)AAsset_getBuffer(asset);
    struct ObjAsset obj;
    obj.length = length1;
    obj.rawData = rawData;
    AssetsFetcher__LoadObjAndConvert(bmin, bmax, filename, &obj, outAttrib);
    AAsset_close(asset);
}

// https://github.com/syoyo/tinyobjloader-c/blob/master/examples/viewer/viewer.c
void get_file_data(
        void* ctx, const char* filename, const int is_mtl,
        const char* obj_filename, char** data, size_t* len) {
    struct ObjAsset *obj = (struct ObjAsset *)ctx;
    *data = (char*)obj->rawData;
    *len = obj->length;
}

// bmax, bmin give the dimensions in order to scale the object in the world
int AssetsFetcher__LoadObjAndConvert(float bmin[3], float bmax[3], const char* filename, struct ObjAsset* obj, tinyobj_attrib_t *outAttrib) {
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;
    {
        unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
        int ret =
                tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials,
                                  &num_materials, filename, get_file_data, (void*)obj, flags);
        if (ret != TINYOBJ_SUCCESS) {
            return 0;
        }

        printf("# of shapes    = %d\n", (int)num_shapes);
        printf("# of materials = %d\n", (int)num_materials);

        /*
        {
          int i;
          for (i = 0; i < num_shapes; i++) {
            printf("shape[%d] name = %s\n", i, shapes[i].name);
          }
        }
        */
    }

    bmin[0] = bmin[1] = bmin[2] = FLT_MAX;
    bmax[0] = bmax[1] = bmax[2] = -FLT_MAX;

    {
        float* vb;
        size_t face_offset = 0;
        size_t i;

        size_t num_triangles = attrib.num_face_num_verts;
        size_t stride = 9;

        //vb = (float*)malloc(sizeof(float) * stride * num_triangles * 3);

        for (i = 0; i < attrib.num_face_num_verts; i++) {
            size_t f;
            assert(attrib.face_num_verts[i] % 3 ==
                   0);
            for (f = 0; f < (size_t)attrib.face_num_verts[i] / 3; f++) {
                size_t k;
                float v[3][3];
                float n[3][3];
                float c[3];
                float len2;

                tinyobj_vertex_index_t idx0 = attrib.faces[face_offset + 3 * f + 0];
                tinyobj_vertex_index_t idx1 = attrib.faces[face_offset + 3 * f + 1];
                tinyobj_vertex_index_t idx2 = attrib.faces[face_offset + 3 * f + 2];

                for (k = 0; k < 3; k++) {
                    int f0 = idx0.v_idx;
                    int f1 = idx1.v_idx;
                    int f2 = idx2.v_idx;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);

                    v[0][k] = attrib.vertices[3 * (size_t)f0 + k];
                    v[1][k] = attrib.vertices[3 * (size_t)f1 + k];
                    v[2][k] = attrib.vertices[3 * (size_t)f2 + k];
                    bmin[k] = (v[0][k] < bmin[k]) ? v[0][k] : bmin[k];
                    bmin[k] = (v[1][k] < bmin[k]) ? v[1][k] : bmin[k];
                    bmin[k] = (v[2][k] < bmin[k]) ? v[2][k] : bmin[k];
                    bmax[k] = (v[0][k] > bmax[k]) ? v[0][k] : bmax[k];
                    bmax[k] = (v[1][k] > bmax[k]) ? v[1][k] : bmax[k];
                    bmax[k] = (v[2][k] > bmax[k]) ? v[2][k] : bmax[k];
                }
/*
                if (attrib.num_normals > 0) {
                    int f0 = idx0.vn_idx;
                    int f1 = idx1.vn_idx;
                    int f2 = idx2.vn_idx;
                    if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                        assert(f0 < (int)attrib.num_normals);
                        assert(f1 < (int)attrib.num_normals);
                        assert(f2 < (int)attrib.num_normals);
                        for (k = 0; k < 3; k++) {
                            n[0][k] = attrib.normals[3 * (size_t)f0 + k];
                            n[1][k] = attrib.normals[3 * (size_t)f1 + k];
                            n[2][k] = attrib.normals[3 * (size_t)f2 + k];
                        }
                    } else {
                        CalcNormal(n[0], v[0], v[1], v[2]);
                        n[1][0] = n[0][0];
                        n[1][1] = n[0][1];
                        n[1][2] = n[0][2];
                        n[2][0] = n[0][0];
                        n[2][1] = n[0][1];
                        n[2][2] = n[0][2];
                    }
                } else {
                    CalcNormal(n[0], v[0], v[1], v[2]);
                    n[1][0] = n[0][0];
                    n[1][1] = n[0][1];
                    n[1][2] = n[0][2];
                    n[2][0] = n[0][0];
                    n[2][1] = n[0][1];
                    n[2][2] = n[0][2];
                }

                for (k = 0; k < 3; k++) {
                    vb[(3 * i + k) * stride + 0] = v[k][0];
                    vb[(3 * i + k) * stride + 1] = v[k][1];
                    vb[(3 * i + k) * stride + 2] = v[k][2];
                    vb[(3 * i + k) * stride + 3] = n[k][0];
                    vb[(3 * i + k) * stride + 4] = n[k][1];
                    vb[(3 * i + k) * stride + 5] = n[k][2];

                    c[0] = n[k][0];
                    c[1] = n[k][1];
                    c[2] = n[k][2];
                    len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                    if (len2 > 0.0f) {
                        float len = (float)sqrt((double)len2);

                        c[0] /= len;
                        c[1] /= len;
                        c[2] /= len;
                    }

                    vb[(3 * i + k) * stride + 6] = (c[0] * 0.5f + 0.5f);
                    vb[(3 * i + k) * stride + 7] = (c[1] * 0.5f + 0.5f);
                    vb[(3 * i + k) * stride + 8] = (c[2] * 0.5f + 0.5f);
                }
                        */
            }

            face_offset += (size_t)attrib.face_num_verts[i];
        }
        //free(vb);
    }

    printf("bmin = %f, %f, %f\n", (double)bmin[0], (double)bmin[1],
           (double)bmin[2]);
    printf("bmax = %f, %f, %f\n", (double)bmax[0], (double)bmax[1],
           (double)bmax[2]);

    //tinyobj_attrib_free(&attrib);
    *outAttrib = attrib;
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return 1;
}

void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
    float v10[3];
    float v20[3];
    float len2;

    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v20[1] * v10[2] - v20[2] * v10[1];
    N[1] = v20[2] * v10[0] - v20[0] * v10[2];
    N[2] = v20[0] * v10[1] - v20[1] * v10[0];

    len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len2 > 0.0f) {
        float len = (float)sqrt((double)len2);

        N[0] /= len;
        N[1] /= len;
    }
}
