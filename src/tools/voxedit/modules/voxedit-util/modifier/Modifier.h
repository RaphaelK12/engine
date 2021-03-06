/**
 * @file
 */

#pragma once

#include "video/ShapeBuilder.h"
#include "render/ShapeRenderer.h"
#include "core/GLM.h"
#include "core/IComponent.h"
#include "video/Camera.h"
#include "voxel/Voxel.h"
#include "math/Axis.h"
#include "voxel/Face.h"
#include "voxel/RawVolume.h"
#include "ModifierType.h"
#include "ModifierButton.h"
#include "math/AABB.h"
#include "Selection.h"
#include "ModifierVolumeWrapper.h"

namespace voxedit {

enum ShapeType {
	AABB,
	Torus,
	Cylinder,
	Cone,
	Dome,
	Ellipse,

	Max
};

class Modifier : public core::IComponent {
private:
	Selection _selection = voxel::Region::InvalidRegion;
	bool _selectionValid = false;
	bool _secondPosValid = false;
	bool _aabbMode = false;
	bool _center = false;
	glm::ivec3 _aabbFirstPos;
	glm::ivec3 _aabbSecondPos;
	math::Axis _aabbSecondActionDirection = math::Axis::None;
	ModifierType _modifierType = ModifierType::Place;
	video::ShapeBuilder _shapeBuilder;
	render::ShapeRenderer _shapeRenderer;
	int32_t _aabbMeshIndex = -1;
	int32_t _selectionIndex = -1;
	int _gridResolution = 1;
	int32_t _mirrorMeshIndex = -1;
	math::Axis _mirrorAxis = math::Axis::None;
	glm::ivec3 _mirrorPos {0};
	glm::ivec3 _cursorPosition {0};
	voxel::FaceNames _face = voxel::FaceNames::NoOfFaces;
	voxel::Voxel _cursorVoxel;
	int32_t _voxelCursorMesh = -1;
	ModifierButton _actionExecuteButton;
	ModifierButton _deleteExecuteButton;
	ShapeType _shapeType = ShapeType::AABB;

	glm::ivec3 firstPos() const;
	bool getMirrorAABB(glm::ivec3& mins, glm::ivec3& maxs) const;
	void updateMirrorPlane();
	void renderAABBMode(const video::Camera& camera);
	void updateSelectionBuffers();
	bool executeShapeAction(ModifierVolumeWrapper& wrapper, const glm::ivec3& mins, const glm::ivec3& maxs, const std::function<void(const voxel::Region& region, ModifierType type)>& callback);
	bool select(const glm::ivec3& mins, const glm::ivec3& maxs, voxel::RawVolume* volume, const std::function<void(const voxel::Region& region, ModifierType type)>& callback);
public:
	Modifier();

	void construct() override;
	bool init() override;
	void shutdown() override;

	void translate(const glm::ivec3& v);

	/**
	 * @return @c true if the modifier aabb selection is not yet done, but
	 * active already
	 */
	bool secondActionMode() const;
	/**
	 * @return The axis along which the aabb may still be modified
	 */
	math::Axis secondActionDirection() const;
	/**
	 * @return @c true if the aabb that was formed has a side that is only 1 voxel
	 * high. This is our indicator for allowing to modify the aabb according to
	 * it's detected axis
	 * @sa secondActionDirection()
	 */
	bool needsSecondAction();
	const Selection& selection() const;

	/**
	 * @brief The modifier can build the aabb from the center of the current
	 * cursor position.
	 * Set this to @c true to activate this. The default is to build the aabb
	 * from the corner(s)
	 */
	void setCenterMode(bool center);
	bool centerMode() const;

	math::Axis mirrorAxis() const;
	void setMirrorAxis(math::Axis axis, const glm::ivec3& mirrorPos);

	ModifierType modifierType() const;
	void setModifierType(ModifierType type);

	const voxel::Voxel& cursorVoxel() const;
	void setCursorVoxel(const voxel::Voxel& voxel);

	ShapeType shapeType() const;
	void setShapeType(ShapeType type);

	bool aabbMode() const;
	glm::ivec3 aabbDim() const;
	glm::ivec3 aabbPosition() const;

	/**
	 * @brief Pick the start position of the modifier execution bounding box
	 */
	bool aabbStart();
	/**
	 * @brief End the current ModifierType execution and modify tha given volume according to the type.
	 * @param[out,in] volume The volume to modify
	 * @param callback Called for every region that was modified for the current active modifier.
	 */
	bool aabbAction(voxel::RawVolume* volume, const std::function<void(const voxel::Region& region, ModifierType type)>& callback);
	void aabbStop();
	void aabbStep();

	bool modifierTypeRequiresExistingVoxel() const;

	void setCursorPosition(const glm::ivec3& pos, voxel::FaceNames face);
	const glm::ivec3& cursorPosition() const;
	voxel::FaceNames cursorFace() const;

	/**
	 * @note Mirrored REMOVE ME
	 */
	void setGridResolution(int resolution);

	void render(const video::Camera& camera);
};

inline bool Modifier::secondActionMode() const {
	return _secondPosValid;
}

inline math::Axis Modifier::secondActionDirection() const {
	return _aabbSecondActionDirection;
}

inline bool Modifier::centerMode() const {
	return _center;
}

inline void Modifier::setCenterMode(bool center) {
	_center = center;
}

inline ShapeType Modifier::shapeType() const {
	return _shapeType;
}

inline void Modifier::setShapeType(ShapeType type) {
	_shapeType = type;
}

inline voxel::FaceNames Modifier::cursorFace() const {
	return _face;
}

inline const voxel::Voxel& Modifier::cursorVoxel() const {
	return _cursorVoxel;
}

inline const glm::ivec3& Modifier::cursorPosition() const {
	return _cursorPosition;
}

inline const Selection& Modifier::selection() const {
	return _selection;
}

}
