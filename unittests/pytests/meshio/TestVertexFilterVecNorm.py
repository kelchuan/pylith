#!/usr/bin/env python
#
# ======================================================================
#
#                           Brad T. Aagaard
#                        U.S. Geological Survey
#
# {LicenseText}
#
# ======================================================================
#

## @file unittests/pytests/meshio/TestVertexFilterVecNorm.py

## @brief Unit testing of Python VertexFilterVecNorm object.

import unittest

from pylith.meshio.VertexFilterVecNorm import MeshVertexFilterVecNorm
from pylith.meshio.VertexFilterVecNorm import SubMeshVertexFilterVecNorm

# ----------------------------------------------------------------------
class TestMeshVertexFilterVecNorm(unittest.TestCase):
  """
  Unit testing of Python VertexFilterVecNorm object.
  """

  def test_constructor(self):
    """
    Test constructor.
    """
    filter = MeshVertexFilterVecNorm()
    filter._configure()
    return


  def test_initialize(self):
    """
    Test constructor.
    """
    filter = MeshVertexFilterVecNorm()
    filter._configure()
    filter.initialize()
    return


  def test_factory(self):
    """
    Test factory method.
    """
    from pylith.meshio.VertexFilterVecNorm import mesh_output_vertex_filter
    filter = mesh_output_vertex_filter()
    return


# ----------------------------------------------------------------------
class TestSubMeshVertexFilterVecNorm(unittest.TestCase):
  """
  Unit testing of Python VertexFilterVecNorm object.
  """

  def test_constructor(self):
    """
    Test constructor.
    """
    filter = SubMeshVertexFilterVecNorm()
    filter._configure()
    return


  def test_initialize(self):
    """
    Test constructor.
    """
    filter = SubMeshVertexFilterVecNorm()
    filter._configure()
    filter.initialize()
    return


  def test_factory(self):
    """
    Test factory method.
    """
    from pylith.meshio.VertexFilterVecNorm import submesh_output_vertex_filter
    filter = submesh_output_vertex_filter()
    return


# End of file 
