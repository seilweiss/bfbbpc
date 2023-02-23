#pragma once

#include "iCamera.h"

#include "xBase.h"
#include "xMath3.h"
#include "xMath2.h"
#include "xBound.h"

#include <rwcore.h>

struct xScene;
struct xEnt;

struct xCamera : xBase
{
    RwCamera* lo_cam;
    xMat4x3 mat;
    xMat4x3 omat;
    xMat3x3 mbasis;
    xBound bound;
    xMat4x3* tgt_mat;
    xMat4x3* tgt_omat;
    xBound* tgt_bound;
    xVec3 focus;
    xScene* sc;
    xVec3 tran_accum;
    F32 fov;
    U32 flags;
    F32 tmr;
    F32 tm_acc;
    F32 tm_dec;
    F32 ltmr;
    F32 ltm_acc;
    F32 ltm_dec;
    F32 dmin;
    F32 dmax;
    F32 dcur;
    F32 dgoal;
    F32 hmin;
    F32 hmax;
    F32 hcur;
    F32 hgoal;
    F32 pmin;
    F32 pmax;
    F32 pcur;
    F32 pgoal;
    F32 depv;
    F32 hepv;
    F32 pepv;
    F32 orn_epv;
    F32 yaw_epv;
    F32 pitch_epv;
    F32 roll_epv;
    xQuat orn_cur;
    xQuat orn_goal;
    xQuat orn_diff;
    F32 yaw_cur;
    F32 yaw_goal;
    F32 pitch_cur;
    F32 pitch_goal;
    F32 roll_cur;
    F32 roll_goal;
    F32 dct;
    F32 dcd;
    F32 dccv;
    F32 dcsv;
    F32 hct;
    F32 hcd;
    F32 hccv;
    F32 hcsv;
    F32 pct;
    F32 pcd;
    F32 pccv;
    F32 pcsv;
    F32 orn_ct;
    F32 orn_cd;
    F32 orn_ccv;
    F32 orn_csv;
    F32 yaw_ct;
    F32 yaw_cd;
    F32 yaw_ccv;
    F32 yaw_csv;
    F32 pitch_ct;
    F32 pitch_cd;
    F32 pitch_ccv;
    F32 pitch_csv;
    F32 roll_ct;
    F32 roll_cd;
    F32 roll_ccv;
    F32 roll_csv;
    xVec4 frustplane[12];
};

struct cameraFX;

typedef void(*cameraFXUpdateCallback)(cameraFX*, F32, const xMat4x3*, xMat4x3*);
typedef void(*cameraFXKillCallback)(cameraFX*);

enum cameraFXType
{
    eCameraFXNone,
    eCameraFXZoom,
    eCameraFXShake,
    eCameraFXCount
};

struct cameraFXShake
{
    F32 magnitude;
    xVec3 dir;
    F32 cycleTime;
    F32 cycleMax;
    F32 dampen;
    F32 dampenRate;
    F32 rotate_magnitude;
    F32 radius;
    const xVec3* epicenterP;
    xVec3 epicenter;
    const xVec3* player;
};

enum cameraFXZoomMode
{
    eCameraFXZoomOut,
    eCameraFXZoomIn,
    eCameraFXZoomHold,
    eCameraFXZoomDone
};

struct cameraFXZoom
{
    F32 holdTime;
    F32 vel;
    F32 accel;
    F32 distance;
    U32 mode;
    F32 velCur;
    F32 distanceCur;
    F32 holdTimeCur;
};

struct cameraFX
{
    S32 type;
    S32 flags;
    F32 elapsedTime;
    F32 maxTime;
    union
    {
        cameraFXShake shake;
        cameraFXZoom zoom;
    };
};

#define CAMERAFX_ACTIVE (1<<0)
#define CAMERAFX_DONE (1<<1)

struct cameraFXTableEntry
{
    S32 type;
    cameraFXUpdateCallback func;
    cameraFXKillCallback funcKill;
};

#define CAMERAFX_MAX 10

class xBinaryCamera
{
    struct zone_data
    {
        F32 distance;
        F32 height;
        F32 height_focus;
    };

    struct config
    {
        zone_data zone_rest;
        zone_data zone_above;
        zone_data zone_below;
        F32 move_speed;
        F32 turn_speed;
        F32 stick_speed;
        F32 stick_yaw_vel;
        F32 max_yaw_vel;
        F32 margin_angle;
    };

    config cfg;
    xCamera* camera;
    xQuat cam_dir;
    const xVec3* s1;
    const xVec3* s2;
    F32 s2_radius;
    xVec2 stick_offset;

public:
    void init();
    void start(xCamera& camera);
    void stop();
    void update(F32 dt);

    void set_targets(const xVec3& s1, const xVec3& s2, F32 s2_radius)
    {
        this->s1 = &s1;
        this->s2 = &s2;
        this->s2_radius = s2_radius;
    }

    void render_debug() {}
    void add_tweaks(const char* prefix) {}
};

extern S32 xcam_collis_owner_disable;
extern S32 xcam_do_collis;
extern F32 xcam_collis_radius;
extern F32 xcam_collis_stiffness;
extern F32 gCameraLastFov;
extern cameraFXTableEntry sCameraFXTable[eCameraFXCount];
extern cameraFX sCameraFX[CAMERAFX_MAX];

void xCameraInit(xCamera* cam, U32 width, U32 height);
void xCameraExit(xCamera* cam);
void xCameraReset(xCamera* cam, F32 d, F32 h, F32 pitch);
void SweptSphereHitsCameraEnt(xScene*, xRay3* ray, xQCData* qcd, xEnt* ent, void* data);
void xCameraUpdate(xCamera* cam, F32 dt);
void xCameraBegin(xCamera* cam, S32 clear);
void xCameraFXBegin(xCamera* cam);
cameraFX* xCameraFXAlloc();
void xCameraFXZoomUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m);
void xCameraFXShake(F32 maxTime, F32 magnitude, F32 cycleMax, F32 rotate_magnitude, F32 radius,
                    const xVec3* epicenter, const xVec3* player);
void xCameraFXShakeUpdate(cameraFX* f, F32 dt, const xMat4x3*, xMat4x3* m);
void xCameraFXUpdate(xCamera* cam, F32 dt);
void xCameraFXEnd(xCamera* cam);
void xCameraEnd(xCamera* cam, F32 seconds, S32 update_scrn_fx);
void xCameraShowRaster(xCamera* cam);
void xCameraSetScene(xCamera* cam, xScene* sc);
void xCameraSetTargetMatrix(xCamera* cam, xMat4x3* mat);
void xCameraSetTargetOMatrix(xCamera* cam, xMat4x3* mat);
void xCameraDoCollisions(S32 do_collis, S32 owner);
void xCameraMove(xCamera* cam, U32 flags, F32 dgoal, F32 hgoal, F32 pgoal,
                 F32 tm, F32 tm_acc, F32 tm_dec);
void xCameraMove(xCamera* cam, const xVec3& loc);
void xCameraMove(xCamera* cam, const xVec3& loc, F32 maxSpeed);
void xCameraFOV(xCamera* cam, F32 fov, F32 maxSpeed, F32 dt);
void xCameraLook(xCamera* cam, U32 flags, const xQuat* orn_goal, F32 tm, F32 tm_acc, F32 tm_dec);
void xCameraLookYPR(xCamera* cam, U32 flags, F32 yaw, F32 pitch, F32 roll,
                    F32 tm, F32 tm_acc, F32 tm_dec);
void xCameraRotate(xCamera* cam, const xMat3x3& m, F32 time, F32 accel, F32 decl);
void xCameraRotate(xCamera* cam, const xVec3& v, F32 roll, F32 time, F32 accel, F32 decl);

inline void add_camera_tweaks() {}

inline F32 xCameraGetFOV(const xCamera* cam)
{
    return cam->fov;
}

inline void xCameraSetFOV(xCamera* cam, F32 fov)
{
    cam->fov = fov;
    iCameraSetFOV(cam->lo_cam, fov);
}