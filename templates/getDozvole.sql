SELECT prostorija FROM dozvole_korisnika
LEFT JOIN vrata ON dozvole_korisnika.id_vrata = vrata.id
WHERE id_korisnika = '{{id_korisnika}}'