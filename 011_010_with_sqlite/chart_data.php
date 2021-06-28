<?php
    $db = new SQLite3("./values.sqlite3", SQLITE3_OPEN_READONLY);
    try {
        if (!$db) {
            echo $db->lastErrorMsg();
        }

        $records = array();
        if ($_GET["interval"] == "24-hours") {
            $sqlite_result = $db->query("SELECT * FROM `values` "
                . "WHERE timestamp BETWEEN datetime('now', '-1 days', 'localtime') AND datetime('now', 'localtime')");
        } else if ($_GET["interval"] == "7-days") {
            $sqlite_result = $db->query("SELECT * FROM `values` "
                . "WHERE timestamp BETWEEN datetime('now', '-7 days', 'localtime') AND datetime('now', 'localtime')");
        } else {
            $sqlite_result = $db->query("SELECT * FROM `values` "
                . "WHERE timestamp BETWEEN datetime('now', '-1 months', 'localtime') AND datetime('now', 'localtime')");
        }
        while ($result = $sqlite_result->fetchArray(SQLITE3_ASSOC)) {
            array_push($records, $result);
        }
        echo json_encode(array("success" => count($records) > 0, "data" => $records));
    } catch (Exception $e) {
        echo $e->getMessage();
    } finally {
        $db->close();
    }
        
