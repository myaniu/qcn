<?
  class SqlQueryString {
    var $table;
    var $query;
    var $urlquery;

    function SqlQueryString() {
        if (isset($_GET['table'])) {
            $this->table = $_GET['table'];
        } else {
            $this->table = "";
        }
        //$this->query = $_GET['query'];
        $this->query = "";
        $this->urlquery = "";
    }
    function add($clause) {
        //$clause=boinc_real_escape_string($clause);
        if (!$this->query) {
            $this->query .= "where $clause";
        } else {
            $this->query .= " and $clause";
        }
    }
    function addclause($clause) {
        if ($clause) {
            $c = urldecode($clause);
            $this->add("( $c )");
            $clause = urlencode($clause);
            $this->urlquery .= "&clauses=$clause";
        }
    }
    function addeq($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = "";
        }
        if (strlen($value)) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addeq_not_CHOOSE_ALL($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = "";
        }
        // On the selection menu, the ALL selection criteria sets (for example)
        // outcome=='CHOOSE_ALL' rather than a numeric value.  This means that
        // we enter no condition for this case.
        if (strlen($value) && strcmp("CHOOSE_ALL", $value)) {
            $this->add("$name = '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addgt($name) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value = '';
        }
        if (strlen($value) && $value > 0) {
            $this->add("$name > '$value'");
            $this->urlquery .= "&$name=".urlencode($value);
        }
    }
    function addsort($name, $order) {
        if (isset($_GET[$name])) {
            $value = $_GET[$name];
        } else {
            $value=null;
        }
        if (isset($_GET[$order])) {
             $order = $_GET[$order];
        } else {
             $order = null;
        }
        if (strlen($value)) {
            if ($order == 'asc') {
                $this->query .= " order by $value asc";
                $this->urlquery .= "&sort_by_order=".urlencode($order);
            } else {
                $this->query .= " order by $value desc";
                $this->urlquery .= "&$name=".urlencode($value);
            }
        }
    }

    function count() {
        $count_query = "select count(*) as cnt from $this->table $this->query";
        $result = mysql_query($count_query);
        if (!$result) return 0;
        $res = mysql_fetch_object($result);
        mysql_free_result($result);
        return $res->cnt;
    }

    function get_select_query($entries_to_show, $start_at) {
        if ($entries_to_show) {
            if ($start_at) {
                return "select * from $this->table $this->query limit $start_at,$entries_to_show";
            } else {
                return "select * from $this->table $this->query limit $entries_to_show";
            }
        } else {
            return "select * from $this->table $this->query";
        }
    }

    function get_url($base = "db_action.php") {
        $s = $base . "?table=$this->table$this->urlquery";
        return $s;
    }

    function process_form_items() {
        $this->addeq('id');
        $this->addeq('platformid');
        $this->addeq('appid');
        $this->addeq('workunitid');
        $this->addeq('hostid');
        $this->addeq('userid');
        $this->addeq('teamid');
        $this->addeq('exit_status');
        if (isset($_GET['nsecs'])) {
            $_GET['mod_time'] = date("YmdHis",time() - $_GET['nsecs']);
        }
        $this->addgt('mod_time');
        $this->addeq_not_CHOOSE_ALL('server_state');
        $this->addeq_not_CHOOSE_ALL('outcome');
        $this->addeq_not_CHOOSE_ALL('client_state');
        $this->addeq_not_CHOOSE_ALL('validate_state');
        $clauses = get_str("clauses", true);
        if (strstr($clauses, ";")) error_page("bad clause");
        if ($clauses) {
            $this->addclause($clauses);
        }
        $this->addsort('sort_by', 'sort_by_order');
    }
  }
?>
