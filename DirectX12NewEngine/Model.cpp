#include "Model.h"

Model::Model()
{
	this->deleteInObject = false;
	ModelType = _Model;
}

Model::~Model()
{
	delete[] subMeshes;
}

AnimatedModel::AnimatedModel(Graphics* gfx)
{
	this->deleteInObject = false;
	ModelType = _AnimatedModel;
}


/*
//TODO : REDO THIS, THIS IS SHIT
std::pair<uint32_t, float> getTimeFraction(const std::vector<float>& times, float& dt) {
    uint32_t segment = 0;
    while (dt > times[segment]) {
        segment++;
    }
    float start = times[segment - 1];
    float end = times[segment];
    float frac = (dt - start) / (end - start);
    return { segment, frac };
}

void AnimatedModel::setPose()
{
    std::pair<uint32_t, float> fp;
    float nTime = time * currentAnimation->tick;
    DirectX::XMMATRIX position, rotation, scale;

    SkeletalAnimationConstantBuffer constantBufferData;
    nTime = fmod(nTime, currentAnimation->lenght);
    if (nTime == 0) { nTime += 0.00000001f; }

    for (int i = 0; i < skeleton.size(); i++) {
        Bone& CurrentBone = skeleton[i];
        KeyFrames keyframe = currentAnimation->keyframes[CurrentBone.name];
        {
            //calculate position
            fp = getTimeFraction(keyframe.positionTimestamps, nTime);
            DirectX::XMFLOAT3 pos1 = keyframe.positions[fp.first - 1];
            DirectX::XMFLOAT3 pos2 = keyframe.positions[fp.first];
            position = DirectX::XMMatrixTranslationFromVector(
                DirectX::XMVectorLerp(
                    DirectX::XMLoadFloat3(&pos1),
                    DirectX::XMLoadFloat3(&pos2),
                    fp.second
                )
            );
        }   
        {
            //calculate rotation
            fp = getTimeFraction(keyframe.rotationTimestamps, nTime);
            DirectX::XMVECTOR rot1 = DirectX::XMLoadFloat4(&keyframe.rotations[fp.first - 1]);
            DirectX::XMVECTOR rot2 = DirectX::XMLoadFloat4(&keyframe.rotations[fp.first]);
            rotation = DirectX::XMMatrixRotationQuaternion(
                DirectX::XMQuaternionSlerp(rot1, rot2, fp.second
                )
            );
        }
        {
            //calculate scale
            fp = getTimeFraction(keyframe.scaleTimestamps, nTime);
            DirectX::XMFLOAT3 scale1 = keyframe.scale[fp.first - 1];
            DirectX::XMFLOAT3 scale2 = keyframe.scale[fp.first];
            scale = DirectX::XMMatrixScalingFromVector(
                DirectX::XMVectorLerp(DirectX::XMLoadFloat3(&scale1), DirectX::XMLoadFloat3(&scale2), fp.second)
                );
        }
        DirectX::XMMATRIX pt = DirectX::XMMatrixIdentity();
        if (CurrentBone.parentIndex != -1) {
            pt = skeleton[CurrentBone.parentIndex].FinalTransformation;
        }
        DirectX::XMMATRIX str = (scale * rotation);
        DirectX::XMMATRIX mat = DirectX::XMMatrixTranspose(str * position);
        CurrentBone.FinalTransformation = pt * mat;
        constantBufferData.SkeletalMatrix[i] = CurrentBone.FinalTransformation * DirectX::XMMatrixTranspose(CurrentBone.inverseBindPoseMatrix);
    }
    updateConstantBuffer(constantBufferData.SkeletalMatrix, skeletalConstantBuffer);
}
*/
