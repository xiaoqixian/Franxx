/**********************************************
  > Project         : Franxx
  > File Name		: DBUtil.java
  > Author			: lunar
  > Email			: lunar_ubuntu@qq.com
  > Created Time	: Wed 12 Aug 2020 09:48:50 PM CST
 **********************************************/

/*
 * 数据库单元
 *
 * 通过连接池来管理连接，接受SQL语句并执行
 */

//不喜欢那种com.franxx.util的包命名方式，搞几层文件夹
package util;

import java.lang.reflect.Field;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

public class DBUtil {
    //vector具有线程同步性
    static Vector<Connection> connectionPool = new Vector<>();

    //类在第一次实例化时需要创建连接，并初始化连接池
    static {
        for (int i = 0; i < 20; i++) {
            try {
                Class.forName("com.sql.jdbc.Driver");
                Connection con = DriverManager.getConnection("jdbc:mysql://localhost:3306/franxx", "root", "lunar");
                connectionPool.add(con);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public static Connection getConnection() {
        Connection con = connectionPool.get(0);
        connectionPool.remove(0);
        return con;
    }

    public static void releaseConnection(Connection con) {
        connectionPool.add(conn);
    }

    public static List query(Class<?> c, String sql, Object... obj) {
        Connection con = getConnection();
        List<Object> list = new ArrayList<>();

        try {
            //填充sql
            PreparedStatement pre = con.prepareStatement(sql);
            for (int i = 0; i < obj.length; i++) {
                pre.setObject(i+1, obj[i]);
            }
            ResultSet res = pre.executeQuery();
            //metaData包含了列信息
            ResultSetMetaData rsmd = res.getMetaData();
            int count = rsmd.getColumnCount();

            while (res.next()) {
                Object obj = c.newInstance();
                for (int i = 1; i <= count; i++) {
                    String fieldName = rsmd.getColumnLabel(i);
                    Field field = c.getDeclaredField(fieldName);
                    field.setAccessible(true);
                    field.set(obj, res.getObject(i));
                }
                list.add(obj);
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            releaseConnection(conn);
        }
        return list;
    }

    public static int update(String sql, Object... obj) {
        Connection conn = getConnection();
        int n = 0;
        try {
            PreparedStatement pre = conn.prepareStatement(sql);
            for (int i = 0; i < obj.length; i++) {
                pre.setObject(i+1, obj[i]);
            }
            n = pre.executeUpdate();
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            releaseConnection(conn);
        }
        return n;
    }
}
