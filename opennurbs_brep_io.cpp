//
// Copyright (c) 1993-2022 Robert McNeel & Associates. All rights reserved.
// OpenNURBS, Rhinoceros, and Rhino3D are registered trademarks of Robert
// McNeel & Associates.
//
// THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
// ALL IMPLIED WARRANTIES OF FITNESS FOR ANY PARTICULAR PURPOSE AND OF
// MERCHANTABILITY ARE HEREBY DISCLAIMED.
//				
// For complete openNURBS copyright information see <http://www.opennurbs.org>.
//
////////////////////////////////////////////////////////////////

#include "opennurbs.h"

#if !defined(ON_COMPILING_OPENNURBS)
// This check is included in all opennurbs source .c and .cpp files to insure
// ON_COMPILING_OPENNURBS is defined when opennurbs source is compiled.
// When opennurbs source is being compiled, ON_COMPILING_OPENNURBS is defined 
// and the opennurbs .h files alter what is declared and how it is declared.
#error ON_COMPILING_OPENNURBS must be defined when compiling opennurbs
#endif

bool
ON_BrepVertex::Write( ON_BinaryArchive& file ) const
{
  bool rc = file.WriteInt( m_vertex_index );
  if ( rc )
    rc = file.WritePoint( point );
  if ( rc )
    rc = file.WriteArray( m_ei );
  if ( rc )
    rc = file.WriteDouble( m_tolerance );
  return rc;
}

bool
ON_BrepVertex::Read( ON_BinaryArchive& file )
{
  bool rc = file.ReadInt( &m_vertex_index );
  if ( rc )
    rc = file.ReadPoint( point );
  if ( rc )
    rc = file.ReadArray( m_ei );
  if ( rc )
    rc = file.ReadDouble( &m_tolerance );
  return rc;
}

bool ON_BrepEdge::Write( ON_BinaryArchive& file ) const
{
  bool rc = file.WriteInt( m_edge_index );
  if (rc) rc = file.WriteInt( m_c3i );
  int i = ProxyCurveIsReversed() ? 1 : 0;
  if (rc) rc = file.WriteInt( i );
  if (rc) rc = file.WriteInterval( ProxyCurveDomain() );
  if (rc) rc = file.WriteInt( 2, m_vi );
  if (rc) rc = file.WriteArray( m_ti );
  if (rc) rc = file.WriteDouble( m_tolerance );
  if ( file.Archive3dmVersion() >= 3 )
  {
    // added in opennurbs version 200206180
    if (rc)
      rc = file.WriteInterval( Domain() );
  }
  return rc;
}

bool ON_BrepEdge::Read( ON_BinaryArchive& file )
{
  int bReversed = false;
  ON_Interval proxy_domain;
  bool rc = file.ReadInt( &m_edge_index );
  if (rc) rc = file.ReadInt( &m_c3i );
  if (rc) rc = file.ReadInt( &bReversed );
  if (rc) rc = file.ReadInterval( proxy_domain );
  if (rc) rc = file.ReadInt( 2, m_vi );
  if (rc) rc = file.ReadArray( m_ti );
  if (rc) rc = file.ReadDouble( &m_tolerance );
  ON_Interval domain = proxy_domain;
  if (    file.Archive3dmVersion() >= 3 
       && file.ArchiveOpenNURBSVersion() >= 200206180 )
  {
    if (rc) 
    {
      rc = file.ReadInterval(domain);
      if ( !rc)
        domain = proxy_domain;
    }
  }
  SetProxyCurve( nullptr, proxy_domain );
  if ( bReversed )
    ON_CurveProxy::Reverse();
  SetDomain(domain);

  return rc;
}

bool ON_BrepTrim::Write( ON_BinaryArchive& file ) const
{
  ON_3dPoint P(0.0,0.0,0.0);
  bool rc = file.WriteInt( m_trim_index );
  int i;
  if ( rc )
    rc = file.WriteInt( m_c2i );
  if ( rc )
    rc = file.WriteInterval( ProxyCurveDomain() );
  if ( rc )
    rc = file.WriteInt( m_ei );
  if ( rc )
    rc = file.WriteInt( 2, m_vi );
  if ( rc )
    rc = file.WriteInt( m_bRev3d );
  i = m_type;
  if ( rc )
    rc = file.WriteInt( i );
  i = m_iso;
  if ( rc )
    rc = file.WriteInt( i );
  if ( rc )
    rc = file.WriteInt( m_li );
  if ( rc )
    rc = file.WriteDouble( 2, m_tolerance );
  if ( file.Archive3dmVersion() < 3 )
  {
    if ( rc )
      rc = file.WritePoint( P ); // m_P[0] );
    if ( rc )
      rc = file.WritePoint( P ); // m_P[1] );
  }
  else
  {
    // trim proxy curve information added in version 200206180
    if (rc )
      rc = file.WriteInterval( Domain() );
    unsigned char b[24];
    memset(b,0,sizeof(b));
    b[0] = ProxyCurveIsReversed() ? 1 : 0;
    if (rc)
      rc = file.WriteChar(8,b);
    b[0] = 0;
    if (rc)
      rc = file.WriteChar(24,b);
  }
  if ( rc )
    rc = file.WriteDouble( m__legacy_2d_tol );
  if ( rc )
    rc = file.WriteDouble( m__legacy_3d_tol );
  return rc;
}

bool ON_BrepTrim::Read( ON_BinaryArchive& file )
{
  ON_3dPoint P[2];
  int i;
  bool rc = file.ReadInt( &m_trim_index );
  if ( rc )
    rc = file.ReadInt( &m_c2i );
  if ( rc )
  {
    ON_Interval d;
    rc = file.ReadInterval( d );
    if (rc)
    {
      SetProxyCurveDomain(d);
      SetDomain(d);
    }
  }
  if ( rc )
    rc = file.ReadInt( &m_ei );
  if ( rc )
    rc = file.ReadInt( 2, m_vi );
  if ( rc )
  {
    i = m_bRev3d;
    rc = file.ReadInt( &i );
    if (rc)
      m_bRev3d = (i!=0);
  }

  i = unknown;
  if ( rc )
    rc = file.ReadInt( &i );
  switch (i) {
  case unknown:
    m_type = unknown;
    break;
  case boundary:
    m_type = boundary;
    break;
  case mated:
    m_type = mated;
    break;
  case seam:
    m_type = seam;
    break;
  case singular:
    m_type = singular;
    break;
  }

  i = ON_Surface::not_iso;
  if ( rc )
    rc = file.ReadInt( &i );
  switch(i) {
  case ON_Surface::not_iso:
    m_iso = ON_Surface::not_iso;
    break;
  case ON_Surface::x_iso:
    m_iso = ON_Surface::x_iso;
    break;
  case ON_Surface::y_iso:
    m_iso = ON_Surface::y_iso;
    break;
  case ON_Surface::W_iso:
    m_iso = ON_Surface::W_iso;
    break;
  case ON_Surface::S_iso:
    m_iso = ON_Surface::S_iso;
    break;
  case ON_Surface::E_iso:
    m_iso = ON_Surface::E_iso;
    break;
  case ON_Surface::N_iso:
    m_iso = ON_Surface::N_iso;
    break;
  }

  if ( rc )
    rc = file.ReadInt( &m_li );
  if ( rc )
    rc = file.ReadDouble( 2, m_tolerance );
  if ( file.Archive3dmVersion() >= 3 && file.ArchiveOpenNURBSVersion() >= 200206180 )
  {
    // read trim proxy curve information added in version 200206180
    ON_Interval d = ProxyCurveDomain();
    if (rc )
    {
      rc = file.ReadInterval( d );
      if ( !rc )
        d = ProxyCurveDomain();
    }
    unsigned char b[24];
    memset(b,0,sizeof(b));
    bool bProxyCurveIsReversed = false;
    if (rc)
    {
      rc = file.ReadChar(8,b);
      if (rc && b[0] == 1 )
        bProxyCurveIsReversed = true;
    }
    if (rc)
      rc = file.ReadChar(24,b);

    if ( bProxyCurveIsReversed )
      ON_CurveProxy::Reverse();
    SetDomain(d);
  }
  else
  {
    if ( rc )
      rc = file.ReadPoint( P[0] ); //m_P[0] );
    if ( rc )
      rc = file.ReadPoint( P[1] ); //m_P[1] );
  }
  if ( rc )
    rc = file.ReadDouble( &m__legacy_2d_tol );
  if ( rc )
    rc = file.ReadDouble( &m__legacy_3d_tol );
  return rc;
}

bool ON_BrepLoop::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.WriteInt( m_loop_index );
  if (rc)
    rc = file.WriteArray( m_ti );
  i = m_type;
  if (rc)
    rc = file.WriteInt( i );
  if (rc)
    rc = file.WriteInt( m_fi );
  return rc;
}

bool ON_BrepLoop::Read( ON_BinaryArchive& file )
{
  int i;
  bool rc = file.ReadInt( & m_loop_index );
  if (rc)
    rc = file.ReadArray( m_ti );

  i = unknown;
  if (rc)
    rc = file.ReadInt( &i );
  switch(i) {
  case unknown:
    m_type = unknown;
    break;
  case outer:
    m_type = outer;
    break;
  case inner:
    m_type = inner;
    break;
  case slit:
    m_type = slit;
    break;
  }

  if (rc)
    rc = file.ReadInt( &m_fi );
  return rc;
}

bool ON_BrepFace::Write( ON_BinaryArchive& file ) const
{
  bool rc = file.WriteInt( m_face_index );
  if ( rc )
    rc = file.WriteArray( m_li );
  if ( rc )
    rc = file.WriteInt( m_si );
  if ( rc )
    rc = file.WriteInt( m_bRev );
  if ( rc )
    rc = file.WriteInt( m_face_material_channel );
  return rc;
}

bool ON_BrepFace::Read( ON_BinaryArchive& file )
{
  int i;
  bool rc = file.ReadInt( &m_face_index );
  if ( rc )
    rc = file.ReadArray( m_li );
  if ( rc )
    rc = file.ReadInt( &m_si );
  if ( rc )
  {
    i = m_bRev;
    rc = file.ReadInt( &i );
    if ( rc )
      m_bRev = (i!=0);
  }
  if ( rc )
  {
    rc = file.ReadInt( &m_face_material_channel );
    if ( m_face_material_channel < 0 )
      m_face_material_channel = 0;
  }
  return rc;
}

bool ON_BrepVertexArray::Read( ON_BinaryArchive& file )
{
  Empty();
  ON__UINT32 tcode = 0;
  ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
  int count = 0;
  int i;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
  if (rc) {
    if (tcode != TCODE_ANONYMOUS_CHUNK)
      rc = false;
    if (rc) rc = file.Read3dmChunkVersion(&major_version,&minor_version);
    if (rc) {
      if ( major_version==1 ) {
        if (rc) rc = file.ReadInt(&count);
        SetCapacity(count);
        for ( i = 0; i < count && rc ; i++ ) {
          ON_BrepVertex& vertex = AppendNew();
          rc = vertex.Read(file)?true:false;
        }    
      }
      else {
        rc = 0;
      }
    }
    if ( !file.EndRead3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepVertexArray::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
  if (rc) {
    rc = file.Write3dmChunkVersion(1,0);
    const int count = Count();
    if (rc) rc = file.WriteInt( count );
    for ( i = 0; rc && i < count; i++ ) {
      if (rc) rc = m_a[i].Write(file);
    }
    if ( !file.EndWrite3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepEdgeArray::Read( ON_BinaryArchive& file )
{
  Empty();
  ON__UINT32 tcode = 0;
  ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
  int count = 0;
  int i;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
  if (rc) {
    if (tcode != TCODE_ANONYMOUS_CHUNK)
      rc = false;
    if (rc) rc = file.Read3dmChunkVersion(&major_version,&minor_version);
    if (rc) {
      if ( major_version==1 ) {
        if (rc) rc = file.ReadInt(&count);
        SetCapacity(count);
        for ( i = 0; i < count && rc ; i++ ) {
          ON_BrepEdge& edge = AppendNew();
          rc = edge.Read(file) ? true : false;
        }    
      }
      else {
        rc = 0;
      }
    }
    if ( !file.EndRead3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepEdgeArray::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
  if (rc) {
    rc = file.Write3dmChunkVersion(1,0);
    const int count = Count();
    if (rc) rc = file.WriteInt( count );
    for ( i = 0; rc && i < count; i++ ) {
      if (rc) rc = m_a[i].Write(file);
    }
    if ( !file.EndWrite3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepTrimArray::Read( ON_BinaryArchive& file )
{
  Empty();
  ON__UINT32 tcode = 0;
  ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
  int count = 0;
  int i;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
  if (rc) {
    if (tcode != TCODE_ANONYMOUS_CHUNK)
      rc = false;
    if (rc) rc = file.Read3dmChunkVersion(&major_version,&minor_version);
    if (rc) {
      if ( major_version==1 ) {
        if (rc) rc = file.ReadInt(&count);
        SetCapacity(count);
        for ( i = 0; i < count && rc ; i++ ) {
          ON_BrepTrim& trim = AppendNew();
          rc = trim.Read(file)?true:false;
          if ( rc )
          {
            if ( trim.m_trim_index != m_count-1 )
            {
              // 28 May 2013 Dale Lear
              //   Fix http://mcneel.myjetbrains.com/youtrack/issue/RH-18299
              //   Fix bogus index values to prevent crashes.
              ON_ERROR("Invalid value of m_trim_index");
              trim.m_trim_index = m_count-1;
            }
          }
        }    
      }
      else {
        rc = 0;
      }
    }
    if ( !file.EndRead3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepTrimArray::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
  if (rc) {
    rc = file.Write3dmChunkVersion(1,0);
    const int count = Count();
    if (rc) rc = file.WriteInt( count );
    for ( i = 0; rc && i < count; i++ ) {
      if (rc) rc = m_a[i].Write(file);
    }
    if ( !file.EndWrite3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepLoopArray::Read( ON_BinaryArchive& file )
{
  Empty();
  ON__UINT32 tcode = 0;
  ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
  int count = 0;
  int i;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
  if (rc) {
    if (tcode != TCODE_ANONYMOUS_CHUNK)
      rc = false;
    if (rc) rc = file.Read3dmChunkVersion(&major_version,&minor_version);
    if (rc) {
      if ( major_version==1 ) {
        if (rc) rc = file.ReadInt(&count);
        SetCapacity(count);
        for ( i = 0; i < count && rc ; i++ ) {
          ON_BrepLoop& loop = AppendNew();
          rc = loop.Read(file) ? true : false;
        }    
      }
      else {
        rc = 0;
      }
    }
    if ( !file.EndRead3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepLoopArray::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
  if (rc) {
    rc = file.Write3dmChunkVersion(1,0);
    const int count = Count();
    if (rc) rc = file.WriteInt( count );
    for ( i = 0; rc && i < count; i++ ) {
      if (rc) rc = m_a[i].Write(file);
    }
    if ( !file.EndWrite3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepFaceArray::Read( ON_BinaryArchive& file )
{
  Empty();
  ON__UINT32 tcode = 0;
  ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
  int count = 0;
  int i;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
  if (rc) {
    if (tcode != TCODE_ANONYMOUS_CHUNK)
      rc = false;
    if (rc) rc = file.Read3dmChunkVersion(&major_version,&minor_version);
    if (rc) {
      if ( major_version==1 ) 
      {
        if (rc) rc = file.ReadInt(&count);
        SetCapacity(count);
        for ( i = 0; i < count && rc ; i++ ) 
        {
          ON_BrepFace& face = AppendNew();
          rc = face.Read(file)?true:false;
        }    

        if ( minor_version >= 1 )
        {
          // chunk version 1.1 and later has face uuids
          for ( i = 0; i < count && rc; i++ )
          {
            rc = file.ReadUuid( m_a[i].m_face_uuid );
          }

          if (rc && minor_version >= 2)
          {
            // chunk version 1.2 and later has per face colors
            bool bHavePerFaceColors = false;
            rc = file.ReadBool(&bHavePerFaceColors);
            if (rc && bHavePerFaceColors)
            {
              for (i = 0; rc && i < count; i++)
              {
                ON_Color per_face_color = ON_Color::UnsetColor;
                rc = file.ReadColor(per_face_color);
                if (rc && ON_Color::UnsetColor != per_face_color)
                  m_a[i].SetPerFaceColor(per_face_color);
              }
            }
          }
        }
      }
      else 
      {
        rc = 0;
      }
    }
    if ( !file.EndRead3dmChunk() )
      rc = false;
  }
  return rc;
}

bool ON_BrepFaceArray::Write( ON_BinaryArchive& file ) const
{
  int i;
  bool rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
  if (rc) 
  {
    // 1.1 added m_face_uuid
    // 1.2 added m_per_face_color
    const int minor_version = file.Archive3dmVersion() >= 70 ? 2 : 1;
    rc = file.Write3dmChunkVersion(1, minor_version);

    // chunk version 1.0 and later
    const int count = Count();
    if (rc) rc = file.WriteInt( count );
    for ( i = 0; rc && i < count; i++ ) 
    {
      if (rc) rc = m_a[i].Write(file);
    }

    // chunk version 1.1 and later
    for ( i = 0; rc && i < count; i++ )
    {
      rc = file.WriteUuid( m_a[i].m_face_uuid );
    }

    if (rc && minor_version >= 2)
    {
      // 1.2 added optional per face colors
      bool bHavePerFaceColors = false;
      for (i = 0; rc && i < count; i++)
      {
        if (ON_Color::UnsetColor != m_a[i].PerFaceColor())
        {
          bHavePerFaceColors = true;
          break;
        }
      }

      rc = file.WriteBool(bHavePerFaceColors);
      if (rc && bHavePerFaceColors)
      {
        for (i = 0; rc && i < count; i++)
          rc = file.WriteColor(m_a[i].PerFaceColor());
      }
    }

    if ( !file.EndWrite3dmChunk() )
      rc = false;
  }
  return rc;
}


bool ON_Brep::Write( ON_BinaryArchive& file ) const
{
  const ON_Brep* brep = this;
  ON_Brep* v2brep = 0;


  //bool rc = file.Write3dmChunkVersion(3,0); // serialization version
  //bool rc = file.Write3dmChunkVersion(3,1); // added meshes

  int minor_version 
    = (file.Archive3dmVersion() <= 50)
    ? 2
    : 3 // August 9, 2016 m_region_topology added to V6 files.
    ;

  bool rc = file.Write3dmChunkVersion(3,minor_version); // added m_is_solid

  // 2d curves
  if (rc) rc = brep->m_C2.Write(file);

  // 3d curves
  if (rc) rc = brep->m_C3.Write(file);

  // untrimmed surfaces
  if (rc) rc = brep->m_S.Write(file);

  // vertices
  if (rc) rc = brep->m_V.Write(file);

  // edges
  if (rc) rc = brep->m_E.Write(file);

  // trims
  if (rc) rc = brep->m_T.Write(file);

  // loops
  if (rc) rc = brep->m_L.Write(file);

  // faces
  if (rc) rc = brep->m_F.Write(file);

  // bounding box
  if (rc) rc = file.WritePoint( brep->m_bbox.m_min );
  if (rc) rc = file.WritePoint( brep->m_bbox.m_max );

  // end of chunk version 3.0

  if (rc)
  {
    // added for chunk version 3.1
    const int face_count = brep->m_F.Count();
    int fi;
    unsigned char b;

    // write render meshes
    rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
    if ( rc )
    {
      for ( fi = 0; rc && fi < face_count; fi++ ) {
        const auto mesh = file.Save3dmRenderMesh(ON::object_type::brep_object) ? brep->m_F[fi].SharedMesh(ON::render_mesh) : nullptr;
        b = mesh ? 1 : 0;
        rc = file.WriteChar(b);
        if (rc && mesh) {
          rc = file.WriteObject(*mesh);
        }
      }
      if ( !file.EndWrite3dmChunk() )
      {
        rc = false;
      }
    }

    // write analysis meshes
    if (rc) rc = file.BeginWrite3dmChunk( TCODE_ANONYMOUS_CHUNK, 0 );
    if ( rc )
    {
      for ( fi = 0; rc && fi < face_count; fi++ ) {
        const auto mesh = file.Save3dmAnalysisMesh(ON::object_type::brep_object) ? brep->m_F[fi].SharedMesh(ON::analysis_mesh) : nullptr;
        b = mesh ? 1 : 0;
        rc = file.WriteChar(b);
        if (rc && mesh) {
          rc = file.WriteObject(*mesh);
        }
      }
      if ( !file.EndWrite3dmChunk() )
        rc = false;
    }

  }
  // end of chunk version 3.1

  if (rc)
  {
    // use value of "this" m_is_solid to avoid expensive 
    // calculation on the v2brep
    if ( !file.WriteInt( m_is_solid ) )
      rc = false;
  }

  // end of chunk version 3.2

  if (rc)
  {
    const bool bWriteRegionTopology
      = (nullptr != m_region_topology)
      && (m_F.UnsignedCount() > 0)
      && (m_region_topology->m_FS.UnsignedCount() == 2 * m_F.UnsignedCount());

    if (minor_version <= 2)
    {
      if (bWriteRegionTopology && minor_version == 2 && 50 == file.Archive3dmVersion())
        Internal_AttachV5RegionTopologyAsUserData(file);
    }
    else if (minor_version >= 3)
    {
      // begin chunk verson 3.3

      // region topology chunk added
      if (!file.BeginWrite3dmAnonymousChunk(1))
        return false;

      rc = false;
      for (;;)
      {
        if (!file.WriteBool(bWriteRegionTopology))
          break;
        if (bWriteRegionTopology)
        {
          if (!m_region_topology->Write(file))
            break;
        }
        rc = true;
        break;
      }
      if (!file.EndWrite3dmChunk())
        rc = false;
      // end chunk version 3.3
    }
  }

  if ( 0 != v2brep )
    delete v2brep;

  return rc;
}

static
void ReadFillInMissingBoxes( ON_Brep& brep )
{
  // older files did not save bounding box information
  int ti, li, lti, trim_count, loop_count;
  const ON_Curve* c2;
  trim_count = brep.m_T.Count();
  loop_count = brep.m_L.Count();
  for ( ti = 0; ti < trim_count; ti++ )
  {
    ON_BrepTrim& trim = brep.m_T[ti];
    if ( !trim.m_pbox.IsValid() ) 
    {
      c2 = trim.TrimCurveOf();
      if ( c2 )
        trim.m_pbox = c2->BoundingBox();
    }
  }

  for ( li = 0; li < loop_count; li++ )
  {
    ON_BrepLoop& loop = brep.m_L[li];
    if ( !loop.m_pbox.IsValid() ) 
    {
      for ( lti = 0; lti < loop.m_ti.Count(); lti++ )
      {
        ti = loop.m_ti[lti];
        if ( ti >= 0 && ti < trim_count )
          loop.m_pbox.Union( brep.m_T[ti].m_pbox );
      }
    }
  }
}

static const ON_BoundingBox Internal_BestBoundingBox(
  const ON_BoundingBox archive_bbox,
  const ON_BoundingBox new_bbox
)
{
  if (false == new_bbox.IsNotEmpty())
    return archive_bbox; // don't have a new bbox

  if (false == archive_bbox.IsValid())
    return new_bbox;

  if (archive_bbox.Includes(new_bbox, false))
    return new_bbox;

  // The best code currently available calculated new_box.
  // I should be good enough. If it is different
  // from archive then you need to understand WHY it is different
  // and fix the problem at the source - not mask it at read time.
  return new_bbox; // <- Good place to debug bbox reading bugs

}

bool ON_Brep::Read( ON_BinaryArchive& file )
{
  int i;
  int C2_count = 0;
  int C3_count = 0;
  int S_count = 0;
  int major_version = 0;
  int minor_version = 0;
  bool rc = file.Read3dmChunkVersion( &major_version, &minor_version );
  if ( rc && major_version == 2 ) 
  {
    rc = ReadOld200(file,minor_version); // legacy trimmed face
  }
  else if ( rc && major_version == 3 ) 
  {
    // 2d curves
    if (rc) 
      rc = m_C2.Read(file);
    C2_count = m_C2.Count();

    // 3d curves
    if (rc) 
      rc = m_C3.Read(file);
    C3_count = m_C3.Count();

    // untrimmed surfaces
    if (rc) 
      rc = m_S.Read(file);
    S_count = m_S.Count();

    // vertices
    if (rc) 
      rc = m_V.Read(file);

    // edges
    if (rc) 
    {
      rc = m_E.Read(file);
      if (rc) {
        for ( i = 0; i < m_E.Count(); i++ ) {
          ON_BrepEdge& e = m_E[i];
          e.m_brep = this;
          if ( e.m_c3i >= 0 && e.m_c3i < C3_count )
          {
            bool bProxyCurveIsReversed = e.ProxyCurveIsReversed();
            ON_Interval pdom = e.ProxyCurveDomain();
            ON_Interval edom = e.Domain();
            e.SetProxyCurve( m_C3[e.m_c3i], pdom );
            if ( bProxyCurveIsReversed )
              e.ON_CurveProxy::Reverse();
            e.SetDomain(edom);
          }
        }
      }
    }

    // trims
    if (rc) 
    {
      rc = m_T.Read(file);
      if (rc) {
        for ( i = 0; i < m_T.Count(); i++ ) {
          ON_BrepTrim& trim = m_T[i];
          trim.m_brep = this;
          if ( trim.m_c2i >= 0 && trim.m_c2i < C2_count )
          {
            bool bProxyCurveIsReversed = trim.ProxyCurveIsReversed();
            ON_Interval pdom = trim.ProxyCurveDomain();
            ON_Interval tdom = trim.Domain();
            trim.SetProxyCurve( m_C2[trim.m_c2i], pdom );
            if ( bProxyCurveIsReversed )
              trim.ON_CurveProxy::Reverse();
            trim.SetDomain(tdom);
          }
        }
      }
    }

    // loops
    if (rc) 
    {
      rc = m_L.Read(file);
      if ( rc )
      {
        for ( i = 0; i < m_L.Count(); i++ ) 
        {
          m_L[i].m_brep = this;
        }
      }
    }

    // faces
    if (rc) 
    {
      rc = m_F.Read(file);
      if (rc) {
        for ( i = 0; i < m_F.Count(); i++ ) {
          ON_BrepFace& f = m_F[i];
          f.m_brep = this;
          if ( f.m_si >= 0 && f.m_si < S_count )
            f.SetProxySurface(m_S[f.m_si]);
        }
      }
    }

    // bounding box
    if (rc) 
      rc = file.ReadPoint( m_bbox.m_min );
    if (rc) 
      rc = file.ReadPoint( m_bbox.m_max );

    // fill in missing information
    ReadFillInMissingBoxes(*this);

    // end of chunk version 3.0

    if (rc && minor_version >= 1 )
    {
      // added for chunk version 3.1

      ON_Object* obj;
      ON__UINT32 tcode = 0;
      ON__INT64 length_TCODE_ANONYMOUS_CHUNK = 0;
      int fi;
      unsigned char b;

      const int face_count = m_F.Count();

      // read render meshes
      tcode = 0;
      length_TCODE_ANONYMOUS_CHUNK = 0;
      rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
      if ( rc )
      {
        if ( tcode != TCODE_ANONYMOUS_CHUNK )
          rc = false;
        else
        {
          for ( fi = 0; rc && fi < face_count; fi++ ) 
          {
            rc = file.ReadChar(&b);
            if (rc && b) 
            {
              obj = 0;
              rc = file.ReadObject(&obj);
              if ( 0 != obj )
              {
                auto pMesh = ON_Mesh::Cast(obj);
                if (nullptr == pMesh)
                {
                  delete obj;
                }
                else
                {
                  m_F[fi].SetMesh(ON::render_mesh, pMesh);
                }
              }
            }
          }
        }
        if ( !file.EndRead3dmChunk() )
          rc = false;
      }

      if (rc)
      {
        // read analysis meshes
        tcode = 0;
        length_TCODE_ANONYMOUS_CHUNK = 0;
        rc = file.BeginRead3dmBigChunk( &tcode, &length_TCODE_ANONYMOUS_CHUNK );
        if ( rc )
        {
          if ( tcode != TCODE_ANONYMOUS_CHUNK )
            rc = false;
          else
          {
            for ( fi = 0; rc && fi < face_count; fi++ ) 
            {
              rc = file.ReadChar(&b);
              if (rc && b) 
              {
                rc = file.ReadObject(&obj);
                
                if (nullptr != obj)
                {
                  auto pMesh = ON_Mesh::Cast(obj);
                  if (nullptr == pMesh)
                  {
                    delete obj;
                  }
                  else
                  {
                    m_F[fi].SetMesh(ON::analysis_mesh, pMesh);
                  }
                }
              }
            }
          }
          if ( !file.EndRead3dmChunk() )
            rc = false;
        }
      }
    }

    if ( rc && minor_version >= 2 )
    {
      rc =  file.ReadInt( &m_is_solid );
      if ( m_is_solid < 0 || m_is_solid >= 3 )
        m_is_solid = 0;
      if (rc && minor_version >= 3)
      {
        // begin chunk verson 3.3

        // region topology chunk added
        int region_topology_chunk_version = 0;
        if (!file.BeginRead3dmAnonymousChunk(&region_topology_chunk_version))
          rc = false;
        else
        {
          bool rc1 = false;
          for (;;)
          {
            if (region_topology_chunk_version < 1)
              break;

            bool bReadRegionTopology = false;
            if (!file.ReadBool(&bReadRegionTopology))
              break;

            if (bReadRegionTopology)
            {
              m_region_topology = new ON_BrepRegionTopology();
              if (!m_region_topology->Read(file))
              {
                delete m_region_topology;
                m_region_topology = nullptr;
                break;
              }
            }
            rc1 = true;
            break;
          }
          if (!file.EndRead3dmChunk())
            rc1 = false;
          if (false == rc1)
            rc = false;
        }
      }
    }
  }

  if ( file.ArchiveOpenNURBSVersion() < 20021002 )
  {
    m_is_solid = 0;
  }

  // GBA 3-Sept-20 RH-60112
  // In V6 and earlier bounding boxes of faces were bounding box of underlying surface.
  // This can result in enourmous boxes which mess up make2d
  // In V7 bounding boxes use the trim pbox resulting in resonable boxes.
  ////if (file.Archive3dmVersion() < 70 || file.ArchiveOpenNURBSVersion() < 2382395020)
  ////{
  ////  if (!m_bbox.IsEmpty())
  ////  {
  ////    ON_BoundingBox orig_box = m_bbox;
  ////    ClearBoundingBox();
  ////    BoundingBox();                    // compute bounding box
  ////    m_bbox.Intersection(orig_box);
  ////  }
  ////}

  // Dale Lear https://mcneel.myjetbrains.com/youtrack/issue/RH-64277
  // The test used to fix RH-60112 was not adequate and now the set of
  // active v7 3dm files has lots of cases with breps that have
  // oversized bounding boxes by the new standard avter Greg's RH-60112 changes.
  // We have already started writing V8 files and those versions of V8
  // have been used by some of our Discourse early adopters.
  // At this point, I'm going to assume a bounding box read from a V7 or V8 file
  // can be too big. By the time V9 rolls around, the boxes should be cleaned up.
  if (file.Archive3dmVersion() < 90 && m_bbox.IsNotEmpty() )
  {
    const ON_BoundingBox archive_brep_bbox = m_bbox;
    ON_BoundingBox new_brep_bbox = ON_BoundingBox::EmptyBoundingBox;
    const unsigned int face_count = m_F.UnsignedCount();
    for (unsigned fi = 0; fi < face_count; ++fi)
    {
      ON_BrepFace& f = this->m_F[fi];
      const ON_BoundingBox archive_face_bbox = f.m_bbox;
      ON_BoundingBox new_face_bbox = f.InternalFaceBoundingBox(false, false);
      f.m_bbox = Internal_BestBoundingBox(archive_face_bbox, new_face_bbox);
      new_brep_bbox.Union(f.m_bbox);
    }
    m_bbox = Internal_BestBoundingBox(archive_brep_bbox, new_brep_bbox);
  }

  return rc;
}


bool ON_Brep::ReadOld200( ON_BinaryArchive& file, int minor_version )
{
  bool rc = true;

  // read legacy trimmed surface collection from Rhino 2.0

  int face_count = 0;
  int edge_count = 0;
  int loop_count = 0;
  int trim_count = 0;
  int outer_flag = 0;

  ON_BoundingBox bnd_2d_bbox;
  int i, fi, fbi, fbcnt, bti, btcnt, twin_index;
  int ftype_flag, btype_flag, gcon_flag, mono_flag;
  char b;

  if (rc) rc = file.ReadInt( &face_count );
  if (rc) rc = file.ReadInt( &edge_count );
  if (rc) rc = file.ReadInt( &loop_count );
  if (rc) rc = file.ReadInt( &trim_count );

  if ( face_count < 1 || edge_count < 1 || loop_count < 1 || trim_count < 1 )
    rc = false;

  if (rc) rc = file.ReadInt( &outer_flag );
  if (rc) rc = file.ReadPoint( m_bbox.m_min );
  if (rc) rc = file.ReadPoint( m_bbox.m_max );

  // 2d curves
  m_C2.Reserve(trim_count);
  for ( i = 0; rc && i < trim_count; i++ ) {
    ON_PolyCurve* curve = new ON_PolyCurve();
    rc = curve->Read( file )?true:false;
    if ( curve->Count() == 1 ) {
      m_C2.Append( curve->HarvestSegment(0) );
      delete curve;
    }
    else
      m_C2.Append( curve );
  }
  const int c2_count = m_C2.Count();

  // 3d curves
  m_C3.Reserve(edge_count);
  for ( i = 0; rc && i < edge_count; i++ ) {
    ON_PolyCurve* curve = new ON_PolyCurve();
    rc = curve->Read( file )?true:false;
    if ( curve->Count() == 1 ) {
      m_C3.Append( curve->HarvestSegment(0) );
      delete curve;
    }
    else
      m_C3.Append( curve );
  }
  const int c3_count = m_C3.Count();

  // make a new edge for each 3d curve
  m_E.Reserve(c3_count);
  for ( i = 0; i < c3_count && rc; i++ ) 
  {
    NewEdge(i);
  }

  // 3d surfaces
  m_S.Reserve(face_count);
  for ( i = 0; rc && i < face_count; i++ ) {
    ON_NurbsSurface* surface = new ON_NurbsSurface();
    rc = surface->Read( file )?true:false;
    m_S.Append( surface );
  }

  ON_SimpleArray<int> te_index(trim_count);
  ON_SimpleArray<int> te_twin_index(trim_count);

  m_F.Reserve(face_count);
  m_L.Reserve(loop_count);
  m_T.Reserve(trim_count);

  for ( fi = 0; rc && fi < face_count; fi++ ) 
  {
    ftype_flag = 0;
    fbcnt = 0;
    ON_BrepFace& f = NewFace(fi);
    if (rc) rc = file.ReadInt( &i ); // legacy face index
    if (rc) rc = file.ReadInt( &i ); // OBSOLETE f.m_material_index
    int k = f.m_bRev;
    if (rc) rc = file.ReadInt( &k );
    if (rc) f.m_bRev = (k!=0);
    if (rc) rc = file.ReadInt( &ftype_flag );
    if (rc) rc = file.ReadPoint( f.m_bbox.m_min );
    if (rc) rc = file.ReadPoint( f.m_bbox.m_max );
    if (rc) rc = file.ReadInt( &fbcnt);
    if (fbcnt < 1 )
      rc = false;
    for ( fbi = 0; rc && fbi < fbcnt; fbi++ ) {
      btype_flag = 0;
      ON_BrepLoop::TYPE looptype = ON_BrepLoop::unknown;
      if (rc) rc = file.ReadInt( &i ); // legacy loop index
      if (rc) rc = file.ReadInt( &btype_flag );
      switch (btype_flag)
      {
      case 0:
        looptype = ON_BrepLoop::outer;
        break;
      case 1:
        looptype = ON_BrepLoop::inner;
        break;
      case -1:
        looptype = ON_BrepLoop::slit;
        break;
      default:
        looptype = ON_BrepLoop::unknown;
        break;
      }
      if (rc) rc = file.ReadDouble( 2, &bnd_2d_bbox.m_min.x );
      if (rc) rc = file.ReadDouble( 2, &bnd_2d_bbox.m_max.x );
      btcnt = 0;
      if (rc) rc = file.ReadInt( &btcnt );
      if (btcnt < 1 )
        rc = false;
      ON_BrepLoop& bnd = NewLoop(looptype,f);
      for ( bti = 0; rc && bti < btcnt; bti++ ) {
        ON_BrepTrim& trim = NewTrim(false,bnd,m_T.Count());
        te_index.Append(trim.m_trim_index);
        if (rc) rc = file.ReadInt( &i ); // legacy trim index
        if ( trim.m_trim_index != i )
        {
          ON_ERROR("ON_Brep::ReadOld200 - trim.m_trim_index out of synch.");
          //rc = false;
          //break;
        }
        if (rc) rc = file.ReadInt( &twin_index );
        te_twin_index.Append(twin_index);
        b = 0;
        if (rc) rc = file.ReadChar( &b ); // true if legacy trim managed 3d edge
        if (rc) rc = file.ReadInt( &trim.m_ei );
        if ( trim.m_ei < 0 || trim.m_ei >= c3_count )
        {
          trim.m_ei = -1;
          if (b)
          {
            ON_ERROR("ON_Brep::ReadOld201 - trim.m_ei out of range.");
            rc = false;
            break;
          }
        }
        if ( trim.m_trim_index >= 0 && trim.m_trim_index < c2_count )
          trim.m_c2i = trim.m_trim_index;
        else {
          ON_ERROR("ON_Brep::ReadOld200 - trim.m_trim_index out of range.");
          rc = false;
          trim.m_c2i = -1;
          break;
        }
        int bRev3d_as_int = trim.m_bRev3d;
        if (rc) rc = file.ReadInt(&bRev3d_as_int);
        if (rc) trim.m_bRev3d = (bRev3d_as_int!=0);
        if (rc) rc = file.ReadInt(&gcon_flag);
        if (rc) rc = file.ReadInt(&mono_flag);
        if (rc) rc = file.ReadDouble(&trim.m__legacy_3d_tol);
        if (rc) rc = file.ReadDouble(&trim.m__legacy_2d_tol);
      }
    }
  }

  // finish hooking trims to edges
  if (rc) {
    int trim_index;
    for ( i = 0; i < trim_count; i++ ) {
      trim_index = te_index[i];
      if ( trim_index >= 0 && trim_index < m_T.Count() )
        continue;
      twin_index = te_twin_index[i];
      if ( twin_index >= 0 && twin_index < m_T.Count() )
        continue;
      ON_BrepTrim& trim1 = m_T[trim_index];
      ON_BrepTrim& trim2 = m_T[twin_index];
      if ( trim1.m_ei >= 0 && trim1.m_ei < c2_count && trim2.m_ei < 0 )
        trim2.m_ei = trim1.m_ei;
      else if ( trim2.m_ei >= 0 && trim2.m_ei < c2_count && trim1.m_ei < 0 )
        trim1.m_ei = trim2.m_ei;
    }
    for ( i = 0; i < m_T.Count(); i++ ) {
      ON_BrepTrim& trim = m_T[i];
      ON_Curve* tcurve = m_C2[trim.m_c2i];
      trim.SetProxyCurve( tcurve );
      if ( trim.m_ei >= 0 && trim.m_ei < c3_count )
        m_E[trim.m_ei].m_ti.Append(trim.m_trim_index);
    }


    // finish setting flags
    SetTrimIsoFlags();
    SetTrimTypeFlags();

    // create 3d vertex information
    SetVertices();

    // set tols from values in file
    SetTolsFromLegacyValues();

  }
  else {
    Destroy();
  }

  if (rc) {
    // 3d render mesh geometry
    ON_Object* obj;
    for ( i = 0; rc && i < face_count; i++ ) {
      ON_BrepFace& f = m_F[i];
      file.ReadChar(&b);
      if (b) {
        obj = 0;
        rc = (file.ReadObject(&obj)==1)?true:false;

        if (nullptr != obj)
        {
          auto pMesh = ON_Mesh::Cast(obj);
          if (pMesh)
          {
            f.SetMesh(ON::render_mesh, pMesh);
          }
          else
          {
            delete pMesh;
          }
        }
      }
    }

    if ( !rc ) {
      // delete render mesh geometry
      for ( i = 0; i < face_count; i++ ) 
      {
        ON_BrepFace& f = m_F[i];
        f.DestroyMesh(ON::render_mesh);
      }
    }

    if (rc && minor_version >= 1) {
      // 3d analysis mesh geometry
      for ( i = 0; rc && i < face_count; i++ ) {
        ON_BrepFace& f = m_F[i];
        file.ReadChar(&b);
        if (b) 
        {
          obj = 0;
          rc = file.ReadObject(&obj)?true:false;

          if (nullptr != obj)
          {
            auto pMesh = ON_Mesh::Cast(obj);
            if (pMesh)
            {
              f.SetMesh(ON::analysis_mesh, pMesh);
            }
            else
            {
              delete pMesh;
            }
          }
        }
      }
      if ( !rc ) {
        // delete analysis mesh geometry
        for ( i = 0; i < face_count; i++ ) {
          ON_BrepFace& f = m_F[i];
          f.DestroyMesh(ON::analysis_mesh);
        }
      }
    }

    // fill in missing information
    ReadFillInMissingBoxes(*this);

    if (!rc ) {
      ON_ERROR("ON_Brep::ReadOld201() - trouble reading render/analysis meshes");
      rc = true;
    }
  }

  // 22 April 2003:
  //   Use outer_flag to set m_is_solid for closed solids
  //   with outward pointing normals.
  if ( 1 == outer_flag && IsSolid() )
    m_is_solid = 1;

  return rc;
}

bool ON_Brep::ReadOld100( ON_BinaryArchive& file )
{
  // b-rep was written by old Rhino I/O tookit
  int sz, i;

  // 2d curve geometry
  file.ReadInt( &sz );
  if ( sz < 1 ) {
    return false;
  }
  m_C2.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_C2.Append( Read100_BrepCurve( file ) );
  }

  // 3d curve geomery
  file.ReadInt( &sz );
  if ( sz < 1 ) {
    return false;
  }
  m_C3.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_C3.Append( Read100_BrepCurve( file ) );
  }

  // surface geometry
  file.ReadInt( &sz );
  if ( sz < 1 ) {
    return false;
  }
  m_S.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_S.Append( Read100_BrepSurface( file ) );
  }
  /*
    CRhinoChunk* pChunk;
    CRhinoNurbsSurface* pSurface;
    const int typecode, length;
    for ( i = 0; i < sz; i++ ) {
      file.ReadInt( &typecode );
      if ( typecode == TCODE_RHINO_OBJECT_NURBS_SURFACE && length > 0 ) {
        pSurface = (CRhinoNurbsSurface*)pChunk;
      }
      m_S.Append( pSurface );
    }
  */

  // read topology

  // vertices
  file.ReadInt( &sz );
  m_V.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_V.AppendNew();
    m_V[i].Read(file);
  }

  // edges
  file.ReadInt( &sz );
  m_E.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_E.AppendNew();
    m_E[i].Read(file);
  }

  // trims
  file.ReadInt( &sz );
  m_T.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_T.AppendNew();
    m_T[i].Read(file);
  }

  // loops
  file.ReadInt( &sz );
  m_L.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_L.AppendNew();
    m_L[i].Read(file);
  }

  // faces
  file.ReadInt( &sz );
  m_F.Reserve(sz);
  for ( i = 0; i < sz; i++ ) {
    m_F.AppendNew();
    m_F[i].Read(file);
  }

  // bounding box
  file.ReadPoint( m_bbox.m_min );
  file.ReadPoint( m_bbox.m_max );

  // fill in missing information
  ReadFillInMissingBoxes(*this);

  return true;
}

bool ON_Brep::ReadOld101( ON_BinaryArchive& file )
{
  ON_Object*  pO = nullptr;
  ON_Curve*   pC = nullptr;
  ON_Surface* pS = nullptr;
  int i, count;

  // 2d curves
  file.ReadInt( &count );
  m_C2.Reserve(count);
  for ( i = 0; i < count; i++ ) 
  {
    pO = nullptr;
    file.ReadObject( &pO );
    pC = ON_Curve::Cast(pO);
    if ( !pC )
      delete pO; // ERROR!
    m_C2.Append( pC );
    pC = nullptr;
    pO = nullptr;
  }

  // 3d curves
  file.ReadInt( &count );
  m_C3.Reserve(count);
  for ( i = 0; i < count; i++ ) 
  {
    pO = nullptr;
    file.ReadObject( &pO );
    pC = ON_Curve::Cast(pO);
    if ( !pC )
      delete pO; // ERROR!
    m_C3.Append( pC );
    pC = nullptr;
    pO = nullptr;
  }

  // untrimmed surfaces
  file.ReadInt( &count );
  m_S.Reserve(count);
  for ( i = 0; i < count; i++ ) 
  {
    pO = nullptr;
    file.ReadObject( &pO );
    pS = ON_Surface::Cast(pO);
    if ( !pS )
      delete pO; // ERROR!
    m_S.Append( pS );
    pS = nullptr;
    pO = nullptr;
  }

  // vertices
  file.ReadInt( &count );
  m_V.Reserve(count);
  m_V.SetCount(count);
  for ( i = 0; i < count; i++ ) 
  {
    m_V[i].Read(file);
  }

  // edges
  file.ReadInt( &count );
  m_E.Reserve(count);
  m_E.SetCount(count);
  for ( i = 0; i < count; i++ ) 
  {
    ON_BrepEdge& edge = m_E[i];
    edge.Read(file);
    edge.SetProxyCurve( edge.m_c3i >= 0 ? m_C3[edge.m_c3i] : 0 );
    edge.m_brep = this;
  }

  // trims
  file.ReadInt( &count );
  m_T.Reserve(count);
  m_T.SetCount(count);
  for ( i = 0; i < count; i++ ) 
  {
    m_T[i].Read(file);
    ON_BrepTrim& trim = m_T[i];
    trim.SetProxyCurve( trim.m_c2i >= 0 ? m_C2[trim.m_c2i] : 0 );
    trim.m_brep = this;
  }

  // loops
  file.ReadInt( &count );
  m_L.Reserve(count);
  m_L.SetCount(count);
  for ( i = 0; i < count; i++ ) 
  {
    m_L[i].Read(file);
    m_L[i].m_brep = this;
  }

  // faces
  file.ReadInt( &count );
  m_F.Reserve(count);
  m_F.SetCount(count);
  for ( i = 0; i < count; i++ ) 
  {
    ON_BrepFace& face = m_F[i];
    face.Read(file);
    face.SetProxySurface(face.m_si >= 0 ? m_S[face.m_si] : 0);
    face.m_brep = this;
  }

  // bounding box
  file.ReadPoint( m_bbox.m_min );
  file.ReadPoint( m_bbox.m_max );

  // fill in missing information
  ReadFillInMissingBoxes(*this);

  return true;
}

ON_Curve* ON_Brep::Read100_BrepCurve( ON_BinaryArchive& ) const
{
  // TODO - look at old Rhino I/O tookit code and read b-rep curves
  return nullptr;
}

ON_Surface* ON_Brep::Read100_BrepSurface( ON_BinaryArchive& ) const
{
  // TODO - look at old Rhino I/O tookit code and read b-rep surfaces
  return nullptr;
}

