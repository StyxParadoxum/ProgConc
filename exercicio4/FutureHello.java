/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;


//classe runnable
class MyCallable implements Callable<Long> {
  //construtor
  MyCallable() {}
 
  //método para execução
  public Long call() throws Exception {
    long s = 0;
    for (long i=1; i<=100; i++) {
      s++;
    }
    return s;
  }
}

class PrimoCallable implements Callable<Boolean> {
  private int num;
  
  PrimoCallable(int n) {
    this.num = n;
  }
 
  public Boolean call() throws Exception {
    if (num < 2) return false;
    
    for (int i = 2; i <= Math.sqrt(num); i++) {
      if (num % i == 0) return false;
    }
    return true;
  }
}

//classe do método main
public class FutureHello  {

  public static void main(String[] args) {

    Scanner sc = new Scanner(System.in);

    System.out.print("Digite o número de threads: ");
    int nThreads = sc.nextInt();

    System.out.print("Digite o valor N (intervalo [1,N]): ");
    int N = sc.nextInt();

    ExecutorService executor = Executors.newFixedThreadPool(nThreads);
    List<Future<Boolean>> list = new ArrayList<Future<Boolean>>();

    for (int i = 1; i <= N; i++) {
      Callable<Boolean> worker = new PrimoCallable(i);
      Future<Boolean> submit = executor.submit(worker);
      list.add(submit);
    }

    int n_primos = 0;

    for (Future<Boolean> future : list) {
      try {
        if (future.get()) n_primos++;
      } 
      catch (InterruptedException | ExecutionException e) {
        e.printStackTrace();
      }
    }

    System.out.println("O número de primos encontrados no intervalo de 1 a " + N + " = " + n_primos);

    executor.shutdown();
    sc.close();
  }
}
