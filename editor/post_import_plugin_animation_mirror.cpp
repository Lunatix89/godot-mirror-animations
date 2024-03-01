#ifdef TOOLS_ENABLED

#include "post_import_plugin_animation_mirror.h"

#include "scene/3d/skeleton_3d.h"
#include "scene/animation/animation_player.h"

SkeletonProfileHumanoidEx::SkeletonProfileHumanoidEx() {
	bone_counterpart_map["LeftEye"] = "RightEye";
	bone_counterpart_map["RightEye"] = "LeftEye";
	bone_counterpart_map["LeftShoulder"] = "RightShoulder";
	bone_counterpart_map["LeftUpperArm"] = "RightUpperArm";
	bone_counterpart_map["LeftLowerArm"] = "RightLowerArm";
	bone_counterpart_map["LeftHand"] = "RightHand";
	bone_counterpart_map["LeftThumbMetacarpal"] = "RightThumbMetacarpal";
	bone_counterpart_map["LeftThumbProximal"] = "RightThumbProximal";
	bone_counterpart_map["LeftThumbDistal"] = "RightThumbDistal";
	bone_counterpart_map["LeftIndexProximal"] = "RightIndexProximal";
	bone_counterpart_map["LeftIndexIntermediate"] = "RightIndexIntermediate";
	bone_counterpart_map["LeftIndexDistal"] = "RightIndexDistal";
	bone_counterpart_map["LeftMiddleProximal"] = "RightMiddleProximal";
	bone_counterpart_map["LeftMiddleIntermediate"] = "RightMiddleIntermediate";
	bone_counterpart_map["LeftMiddleDistal"] = "RightMiddleDistal";
	bone_counterpart_map["LeftRingProximal"] = "RightRingProximal";
	bone_counterpart_map["LeftRingIntermediate"] = "RightRingIntermediate";
	bone_counterpart_map["LeftRingDistal"] = "RightRingDistal";
	bone_counterpart_map["LeftLittleProximal"] = "RightLittleProximal";
	bone_counterpart_map["LeftLittleIntermediate"] = "RightLittleIntermediate";
	bone_counterpart_map["LeftLittleDistal"] = "RightLittleDistal";
	bone_counterpart_map["RightShoulder"] = "LeftShoulder";
	bone_counterpart_map["RightUpperArm"] = "LeftUpperArm";
	bone_counterpart_map["RightLowerArm"] = "LeftLowerArm";
	bone_counterpart_map["RightHand"] = "LeftHand";
	bone_counterpart_map["RightThumbMetacarpal"] = "LeftThumbMetacarpal";
	bone_counterpart_map["RightThumbProximal"] = "LeftThumbProximal";
	bone_counterpart_map["RightThumbDistal"] = "LeftThumbDistal";
	bone_counterpart_map["RightIndexProximal"] = "LeftIndexProximal";
	bone_counterpart_map["RightIndexIntermediate"] = "LeftIndexIntermediate";
	bone_counterpart_map["RightIndexDistal"] = "LeftIndexDistal";
	bone_counterpart_map["RightMiddleProximal"] = "LeftMiddleProximal";
	bone_counterpart_map["RightMiddleIntermediate"] = "LeftMiddleIntermediate";
	bone_counterpart_map["RightMiddleDistal"] = "LeftMiddleDistal";
	bone_counterpart_map["RightRingProximal"] = "LeftRingProximal";
	bone_counterpart_map["RightRingIntermediate"] = "LeftRingIntermediate";
	bone_counterpart_map["RightRingDistal"] = "LeftRingDistal";
	bone_counterpart_map["RightLittleProximal"] = "LeftLittleProximal";
	bone_counterpart_map["RightLittleIntermediate"] = "LeftLittleIntermediate";
	bone_counterpart_map["RightLittleDistal"] = "LeftLittleDistal";
	bone_counterpart_map["LeftUpperLeg"] = "RightUpperLeg";
	bone_counterpart_map["LeftLowerLeg"] = "RightLowerLeg";
	bone_counterpart_map["LeftFoot"] = "RightFoot";
	bone_counterpart_map["LeftToes"] = "RightToes";
	bone_counterpart_map["RightUpperLeg"] = "LeftUpperLeg";
	bone_counterpart_map["RightLowerLeg"] = "LeftLowerLeg";
	bone_counterpart_map["RightFoot"] = "LeftFoot";
	bone_counterpart_map["RightToes"] = "LeftToes";
}

void PostImportPluginAnimationMirror::get_internal_import_options(InternalImportCategory p_category, List<ResourceImporter::ImportOption> *r_options) {
	EditorScenePostImportPlugin::get_internal_import_options(p_category, r_options);
}

void PostImportPluginAnimationMirror::internal_process(InternalImportCategory p_category, Node *p_base_scene, Node *p_node, Ref<Resource> p_resource, const Dictionary &p_options) {
	EditorScenePostImportPlugin::internal_process(p_category, p_base_scene, p_node, p_resource, p_options);

	if (p_category == INTERNAL_IMPORT_CATEGORY_ANIMATION) {
		if (!static_cast<bool>(p_options["settings/mirror"])) {
			return;
		}

		Animation *animation = Object::cast_to<Animation>(p_resource.ptr());
		if (animation == nullptr) {
			return;
		}

		AnimationPlayer *animation_player = nullptr;

		TypedArray<Node> nodes = p_base_scene->find_children("*", "AnimationPlayer");
		while (nodes.size()) {
			AnimationPlayer *ap = Object::cast_to<AnimationPlayer>(nodes.pop_back());
			if (!ap->has_animation(animation->get_name())) {
				continue;
			}

			// sanity check for same animation name in different players
			if (ap->get_animation(animation->get_name()) != animation) {
				continue;
			}

			animation_player = ap;
			break;
		}

		const int track_count = animation->get_track_count();
		for (int i = 0; i < track_count; i++) {
			mirror_node_path(animation, animation_player, i);

			switch (animation->track_get_type(i)) {
				case Animation::TYPE_POSITION_3D:
					mirror_position_track(animation, i);
					break;
				case Animation::TYPE_ROTATION_3D:
					mirror_rotation_track(animation, i);
					break;
			}
		}
	}
}

PostImportPluginAnimationMirror::PostImportPluginAnimationMirror() {
}

void PostImportPluginAnimationMirror::mirror_node_path(Animation *p_animation, const AnimationPlayer *p_animation_player, const int &p_track_index) const {
	const auto node_path = p_animation->track_get_path(p_track_index);
	if (node_path.get_subname_count() <= 0) {
		return;
	}

	Skeleton3D *skeleton = Object::cast_to<Skeleton3D>(p_animation_player->get_node(p_animation_player->get_root_node())->get_node(node_path));
	ERR_FAIL_COND_MSG(skeleton == nullptr, vformat("Can't mirror animation \"%s\" - no skeleton found for track path \"%s\"", p_animation->get_name(), node_path));

	const auto &bone_name = node_path.get_subname(node_path.get_subname_count() - 1);
	const auto &bone_index = skeleton->find_bone(bone_name);

	if (bone_index < 0) {
		return;
	}

#ifdef USE_NATIVE_BONE_COUNTERPART_NAMES
	// I have a private fork of the engine which has "bone_counterpart_name" directly implemented into SkeletonProfileHumanoid, BoneMap, Skeleton3D and several other places.
	// If I can get this into the Godot engine, we could even let users add counterpart bones for generic rigs by themselves.
	// But for now, we have to live with a hard coded solution.
	const auto &bone_counterpart_name = skeleton->get_bone_counterpart_name(bone_index);
	if (bone_counterpart_name == StringName()) {
		return;
	}
#else
	const auto entry = default_skeleton_profile_humanoid.bone_counterpart_map.find(bone_name);
	if (entry == default_skeleton_profile_humanoid.bone_counterpart_map.end()) {
		// no counterpart for the bone defined
		return;
	}

	const auto &bone_counterpart_name = entry->value;
#endif

	auto sub_names = node_path.get_subnames();
	sub_names.set(sub_names.size() - 1, bone_counterpart_name);

	const auto mirror_path = NodePath(node_path.get_names(), sub_names, node_path.is_absolute());
	p_animation->track_set_path(p_track_index, mirror_path);
}

void PostImportPluginAnimationMirror::mirror_position_track(Animation *p_animation, const int &p_track_index) {
	const auto scale = Vector3(-1, 1, 1);

	for (int i = 0; i < p_animation->track_get_key_count(p_track_index); i++) {
		const auto value = p_animation->track_get_key_value(p_track_index, i);

		if (value.get_type() != Variant::VECTOR3) {
			continue;
		}

		const auto newValue = static_cast<Vector3>(value) * scale;
		p_animation->track_set_key_value(p_track_index, i, newValue);
	}
}

void PostImportPluginAnimationMirror::mirror_rotation_track(Animation *p_animation, const int &p_track_index) {
	for (int i = 0; i < p_animation->track_get_key_count(p_track_index); i++) {
		const auto value = p_animation->track_get_key_value(p_track_index, i);

		if (value.get_type() != Variant::QUATERNION) {
			continue;
		}

		const auto current_rotation = static_cast<Quaternion>(value);
		const auto new_rotation = Quaternion(-current_rotation.x, current_rotation.y, current_rotation.z, -current_rotation.w);

		p_animation->track_set_key_value(p_track_index, i, new_rotation);
	}
}

#endif // TOOLS_ENABLED
