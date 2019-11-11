/* This file is Copyright University of Edinburgh 2018. For license details, see LICENSE. */

#pragma once

#include "genC/DiagNode.h"

namespace gensim
{
	class DiagNode;

	namespace genc
	{
		namespace ssa
		{
			namespace MetadataKind
			{
				enum MetadataKind {
					UNKNOWN,
					CUSTOM,

					WELL_KNOWN_METADATA_START,
					DIAGNOSTIC_NODE
				};
			}

			class SSAValue;

			class SSAMetadata
			{
				friend class SSAValue;

			public:
				SSAMetadata(MetadataKind::MetadataKind kind) : _owner(nullptr), _kind(kind) { }
				virtual ~SSAMetadata() {}

				SSAValue *GetOwner() const
				{
					return _owner;
				}
				MetadataKind::MetadataKind GetKind() const
				{
					return _kind;
				}

			private:
				SSAValue *_owner;
				MetadataKind::MetadataKind _kind;
			};

			class DiagnosticNodeMetadata : public SSAMetadata
			{
			public:
				DiagnosticNodeMetadata(const DiagNode& node) : SSAMetadata(MetadataKind::DIAGNOSTIC_NODE), _node(node) { }

				const DiagNode& GetDiagNode() const
				{
					return _node;
				}

			private:
				const DiagNode _node;
			};
		}
	}
}
