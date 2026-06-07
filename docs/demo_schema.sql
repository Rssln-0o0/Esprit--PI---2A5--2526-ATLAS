-- Oracle : géolocalisation uniquement dans BATEAU (suppression de BATEAU_GPS_TRACE)
-- Exécuter dans SQL Developer connecté en tant que propriétaire des tables (ex. utilisateur PROJET).
--
-- Évite ORA-01430 (colonnes déjà présentes sur BATEAU).
-- Pas de colonne IDENTITY : certaines versions Oracle renvoient ORA-02000 ; on utilise
-- séquence + trigger (compatible 11g et suivantes).

-- ---------------------------------------------------------------------------
-- 1) Colonnes sur BATEAU (une par une, seulement si absentes)
-- ---------------------------------------------------------------------------
SET DEFINE OFF;
DECLARE
    PROCEDURE add_col_if_missing(p_name VARCHAR2, p_type VARCHAR2) IS
        n NUMBER;
    BEGIN
        SELECT COUNT(*)
          INTO n
          FROM user_tab_columns
         WHERE table_name = 'BATEAU'
           AND column_name = UPPER(TRIM(p_name));
        IF n = 0 THEN
            EXECUTE IMMEDIATE 'ALTER TABLE BATEAU ADD (' || p_name || ' ' || p_type || ')';
            DBMS_OUTPUT.PUT_LINE('Colonne ajoutée : ' || p_name);
        ELSE
            DBMS_OUTPUT.PUT_LINE('Colonne déjà présente (ignoré) : ' || p_name);
        END IF;
    END;
BEGIN
    add_col_if_missing('LATITUDE', 'NUMBER(10,7)');
    add_col_if_missing('LONGITUDE', 'NUMBER(10,7)');
    add_col_if_missing('CAP_DERNIER', 'NUMBER(6,2)');
    add_col_if_missing('DERNIER_FIX_GPS', 'TIMESTAMP(6)');
END;
/

COMMENT ON COLUMN BATEAU.LATITUDE IS 'Dernière latitude WGS84 (ex. ESP32)';
COMMENT ON COLUMN BATEAU.LONGITUDE IS 'Dernière longitude WGS84';
COMMENT ON COLUMN BATEAU.CAP_DERNIER IS 'Dernier cap / route vraie en degrés (0–360)';
COMMENT ON COLUMN BATEAU.DERNIER_FIX_GPS IS 'Horodatage du dernier point GPS reçu';

-- Exemple de mise à jour après réception ATLAS_GPS (ID_BATEAU = 1) :
-- UPDATE BATEAU
--    SET LATITUDE = 36.805123,
--        LONGITUDE = 10.175456,
--        CAP_DERNIER = 187.5,
--        DERNIER_FIX_GPS = SYSTIMESTAMP
--  WHERE ID_BATEAU = 1;

-- ---------------------------------------------------------------------------
-- 2) Migration des dernières positions puis nettoyage de BATEAU_GPS_TRACE
-- ---------------------------------------------------------------------------
DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*)
      INTO n
      FROM user_tables
     WHERE table_name = 'BATEAU_GPS_TRACE';
    IF n > 0 THEN
        EXECUTE IMMEDIATE q'[
            MERGE INTO BATEAU b
            USING (
                SELECT t.ID_BATEAU, t.LATITUDE, t.LONGITUDE, t.FIX_TS
                FROM BATEAU_GPS_TRACE t
                JOIN (
                    SELECT ID_BATEAU, MAX(FIX_TS) AS MAX_FIX_TS
                    FROM BATEAU_GPS_TRACE
                    GROUP BY ID_BATEAU
                ) m
                  ON m.ID_BATEAU = t.ID_BATEAU
                 AND m.MAX_FIX_TS = t.FIX_TS
            ) s
               ON (b.ID_BATEAU = s.ID_BATEAU)
            WHEN MATCHED THEN
                UPDATE SET
                    b.LATITUDE = s.LATITUDE,
                    b.LONGITUDE = s.LONGITUDE,
                    b.DERNIER_FIX_GPS = s.FIX_TS
        ]';
        DBMS_OUTPUT.PUT_LINE('Migration des dernières positions BATEAU_GPS_TRACE -> BATEAU effectuée.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Aucune migration: table BATEAU_GPS_TRACE absente.');
    END IF;
END;
/

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*)
      INTO n
      FROM user_triggers
     WHERE trigger_name = 'TRG_BATEAU_GPS_TRACE_BI';
    IF n > 0 THEN
        EXECUTE IMMEDIATE 'DROP TRIGGER TRG_BATEAU_GPS_TRACE_BI';
        DBMS_OUTPUT.PUT_LINE('Trigger TRG_BATEAU_GPS_TRACE_BI supprimé.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Trigger TRG_BATEAU_GPS_TRACE_BI absent (ignoré).');
    END IF;
END;
/

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*)
      INTO n
      FROM user_tables
     WHERE table_name = 'BATEAU_GPS_TRACE';
    IF n > 0 THEN
        EXECUTE IMMEDIATE 'DROP TABLE BATEAU_GPS_TRACE PURGE';
        DBMS_OUTPUT.PUT_LINE('Table BATEAU_GPS_TRACE supprimée.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Table BATEAU_GPS_TRACE absente (ignoré).');
    END IF;
END;
/

DECLARE
    n NUMBER;
BEGIN
    SELECT COUNT(*)
      INTO n
      FROM user_sequences
     WHERE sequence_name = 'SEQ_BATEAU_GPS_TRACE';
    IF n > 0 THEN
        EXECUTE IMMEDIATE 'DROP SEQUENCE SEQ_BATEAU_GPS_TRACE';
        DBMS_OUTPUT.PUT_LINE('Séquence SEQ_BATEAU_GPS_TRACE supprimée.');
    ELSE
        DBMS_OUTPUT.PUT_LINE('Séquence SEQ_BATEAU_GPS_TRACE absente (ignoré).');
    END IF;
END;
/
