--ACCESS=access content
SELECT file_managed.uri
FROM file_managed
JOIN field_data_field_area_featured_image ON field_data_field_area_featured_image.entity_id=:area_id
WHERE field_data_field_area_featured_image.field_area_featured_image_fid = file_managed.fid