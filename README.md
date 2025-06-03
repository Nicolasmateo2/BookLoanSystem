Sistema de Préstamo de Libros

Este proyecto implementa un sistema concurrente para gestionar préstamos, renovaciones y devoluciones de libros en una biblioteca, utilizando procesos e hilos POSIX, pipes nominales para comunicación entre procesos, y mecanismos de sincronización (mutex y variables de condición).  

Componentes Principales
1. Proceso Solicitante ('solicitante') 
   - Permite a los usuarios realizar operaciones mediante:  
     - Menú interactivo (préstamos, renovaciones, devoluciones).  
     - Archivo de texto ('entrada.txt') con formato:  
         
       Operación,Título,ISBN[,Ejemplar]  
         
       Donde 'Operación' puede ser:  
       - 'P': Préstamo  
       - 'R': Renovación  
       - 'D': Devolución  
       - 'Q': Salir  

2. Proceso Receptor ('receptor')
   - Gestiona las solicitudes y actualiza la base de datos ('libros.txt').  
   - Hilo auxiliar: Procesa devoluciones (`D`) y renovaciones ('R') en segundo plano.  
   - Comandos:  
     - '→reporte': Muestra el estado de los libros.  
     - '→salir': Termina el proceso.  

Instrucciones de Uso  
1. Compilación:  
   bash  
   gcc -o receptor main_receptor.c -lpthread  
   gcc -o solicitante main_solicitante.c  
     

2. Ejecución:  
   - Receptor:  
     bash  
     ./receptor -f libros.txt [-v] [-s salida.txt]  
       
     - '-v': Modo verbose (muestra logs detallados).  
     - '-s': Guarda la BD final en 'salida.txt'.  
   - Solicitante:  
     bash  
     ./solicitante  
       
     - Selecciona modo interactivo o desde archivo.  

3. Formato de 'libros.txt':  
     
   Título,ISBN,NúmEjemplares  
   Ejemplar,Estado,Fecha  
     
   - 'Estado': '0' (disponible) o '1' (prestado).  

Ejemplo de Flujo
1. Un usuario solicita un préstamo:  
     
   ✅ Préstamo exitoso: "Cálculo Diferencial" (Ejemplar 1). Devuelva antes del 05-06-2025  
     
2. El receptor actualiza automáticamente 'libros.txt'.  
3. Al usar '→reporte', se muestra:  
   
   P, Cálculo Diferencial, 120, 1, 05-06-2025  
     

---

**Nota**: Para más detalles, consulte el informe PDF adjunto con la estructura del sistema.
