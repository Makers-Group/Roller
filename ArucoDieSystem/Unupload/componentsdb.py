def datos(Host, User, Pw, Datab):{
    db = mysql.connector.connect(									
	host=Host,
	user=User,
	passwd=Pw,
	database=Datab
    )
    cursor = db.cursor()
}

def selectMax(col, tabla):{
    cursor.execute("SELECT max("+col+") FROM "+tabla+")"
    sup = cursor.fetchone()
    db.commit()
    return sup
}

def insertInto2(col1, col2, val1, val2):{
    sql_insert = ("INSERT INTO Pinblock ("+col1+", "+col2+") VALUES (%s, %s)")
    cursor.execute(sql_insert(val1, val2))
    db.commit()
}